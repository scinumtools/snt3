"""The same heat simulation with parameter semantics evaluated by DIPL."""

from pathlib import Path

from scinumtools3.dip import DIP

from core import MATERIAL_NAMES, simulate


def evaluate():
    """Read the evaluated DIPL model and run the shared solver."""
    parser = DIP()
    parser.add_file(Path(__file__).with_name("parameters.dip"))
    parameters = parser.parse()

    run_id = parameters["run.id"].value
    cells = parameters["run.cells"].value
    steps = parameters["run.steps"].value
    dt = parameters["run.dt"].value
    initial = parameters["rod.initial_temperature"].value
    boundary = parameters["rod.boundary_temperature"].value

    factors = {name: parameters[f"run.{name}_fourier"].value for name in MATERIAL_NAMES}
    metadata = parameters["materials[trial].diffusivity"].provenance.metadata
    return simulate(
        run_id=run_id,
        cells=cells,
        steps=steps,
        initial=initial,
        boundary=boundary,
        timestep=dt,
        factors=factors,
        title=metadata.title,
        description=metadata.description,
    )


if __name__ == "__main__":
    print(evaluate())
