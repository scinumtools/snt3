"""Calculate retry schedules from YAML and Pint; no network calls."""

import math
from pathlib import Path

import pint
import yaml

ureg = pint.UnitRegistry()


def quantity(spec, target_unit):
    """Check the input fields and convert a quantity into the expected unit."""
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


def attempt_starts(attempts, timeout, delay):
    """Assume every attempt consumes its timeout, with a delay before each retry."""
    return [i * (timeout + delay) for i in range(attempts)]


def main():
    path = Path(__file__).with_name("parameters.yaml")
    parameters = yaml.safe_load(path.read_text(encoding="utf-8"))

    # This example defines an override convention on top of YAML mappings.
    for target, replacement in parameters.pop("overrides", {}).items():
        section, name, field = target.split(".")
        if (
            section != "clients"
            or name not in parameters[section]
            or field not in parameters[section][name]
        ):
            raise ValueError(f"Unknown override target: {target}")
        original = parameters[section][name][field]
        if isinstance(original, dict) and "provenance" in original:
            replacement = {**replacement, "provenance": original["provenance"]}
        parameters[section][name][field] = replacement

    clients = parameters["clients"]
    evaluated = {}
    for name in ("catalog", "search"):
        policy = clients[name]
        attempts = policy["attempts"]
        if type(attempts) is not int or not 1 <= attempts <= 10:
            raise ValueError("attempts must be an integer between 1 and 10")
        timeout = quantity(policy["timeout"], "s")
        delay = quantity(policy["retry_delay"], "s")
        budget = quantity(policy["budget"], "s")
        if timeout <= 0 or delay < 0 or budget <= 0:
            raise ValueError("timeout and budget must be positive; retry_delay cannot be negative")
        duration = attempts * timeout + (attempts - 1) * delay
        if duration > budget:
            raise ValueError(f"{name}: maximum retry duration exceeds budget")
        evaluated[name] = (attempts, timeout, delay, duration, budget)

    # Validate both policies before calculating or printing any schedule.
    for name, (attempts, timeout, delay, duration, budget) in evaluated.items():
        starts = attempt_starts(attempts, timeout, delay)
        print(f"{name} attempt starts = {', '.join(f'{t:.3f}' for t in starts)} s")
        print(f"{name} max duration = {duration:.3f} s (budget {budget:.3f} s)")

    metadata = clients["search"]["timeout"]["provenance"]
    print(f"parameter title = {metadata['title']}")
    print(f"description = {metadata['description']}")


if __name__ == "__main__":
    main()
