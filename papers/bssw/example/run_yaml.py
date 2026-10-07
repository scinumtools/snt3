"""A small heat simulation with parameter semantics implemented in Python."""

import math
import re
from pathlib import Path

import pint
import yaml

from core import MATERIAL_NAMES, simulate

ureg = pint.UnitRegistry()
MAX_UINT32 = 2**32 - 1


def quantity(spec, target_unit):
    """Check a physical input and let Pint convert it to the required unit."""
    if (
        not isinstance(spec, dict)
        or not {"value", "unit"} <= spec.keys()
        or set(spec) - {"value", "unit", "provenance"}
    ):
        raise ValueError("A quantity needs value and unit fields")
    value = spec["value"]
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value):
        raise ValueError("A quantity needs a finite numerical value")
    return ureg.Quantity(value, spec["unit"]).to(target_unit).magnitude


def positive(value, name):
    if value <= 0:
        raise ValueError(f"{name} must be positive")
    return value


def evaluate():
    """Apply YAML parameter semantics in Python and run the shared solver."""
    path = Path(__file__).with_name("parameters.yaml")
    parameters = yaml.safe_load(path.read_text(encoding="utf-8"))

    # YAML stores these as ordinary mappings; the application gives them meaning.
    for target, replacement in parameters.pop("overrides", {}).items():
        section, name, field = target.split(".")
        if (
            section not in parameters
            or name not in parameters[section]
            or field not in parameters[section][name]
        ):
            raise ValueError(f"Unknown override target: {target}")
        original = parameters[section][name][field]
        if "provenance" in original:
            replacement = {**replacement, "provenance": original["provenance"]}
        parameters[section][name][field] = replacement

    rod, materials, run = (parameters[name] for name in ("rod", "materials", "run"))
    length = positive(quantity(rod["length"], "m"), "length")
    alpha = {
        name: positive(quantity(materials[name]["diffusivity"], "m**2/s"), "diffusivity")
        for name in MATERIAL_NAMES
    }
    initial = positive(quantity(rod["initial_temperature"], "K"), "temperature")
    boundary = positive(quantity(rod["boundary_temperature"], "K"), "temperature")

    cells, steps, safety = run["cells"], run["steps"], run["safety"]
    run_id = run["id"]
    if not isinstance(run_id, str) or re.fullmatch(r"[a-z][a-z0-9_]*", run_id) is None:
        raise ValueError("run.id must start with a lowercase letter and use only lowercase letters, digits, or underscores")
    if type(cells) is not int or not 2 <= cells <= MAX_UINT32:
        raise ValueError("cells must be a uint32 value of at least 2")
    if type(steps) is not int or not 1 <= steps <= MAX_UINT32:
        raise ValueError("steps must be a positive uint32 value")
    if isinstance(safety, bool) or not isinstance(safety, (int, float)):
        raise ValueError("safety must be numerical")
    if not math.isfinite(safety) or not 0 < safety <= 0.5:
        raise ValueError("safety must be between 0 and 0.5")

    dx = length / cells
    dt = safety * dx * dx / alpha["trial"]
    factors = {}
    for name in MATERIAL_NAMES:
        factor = alpha[name] * dt / dx / dx
        if not 0 < factor <= 0.5:
            raise ValueError("unstable timestep")
        factors[name] = factor

    metadata = materials["trial"]["diffusivity"]["provenance"]
    return simulate(
        run_id=run_id,
        cells=cells,
        steps=steps,
        initial=initial,
        boundary=boundary,
        timestep=dt,
        factors=factors,
        title=metadata["title"],
        description=metadata["description"],
    )


if __name__ == "__main__":
    print(evaluate())
