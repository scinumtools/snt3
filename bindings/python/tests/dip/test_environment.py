import pytest

from scinumtools3.dip import DIP, Environment, OutputFormat


@pytest.fixture
def env():
    dip = DIP()
    dip.add_string(
        'title str = "Python environment"\n'
        "simulation\n"
        "  steps int = 100\n"
        "  timestep float = 0.5 fs\n"
        "  enabled bool = true\n"
        "boundary[inlet]\n"
        "  velocity float[3] = [1.0, 0.0, 0.0] m/s\n"
        "samples[]\n"
        "  time float = 0 s\n"
        "samples[]\n"
        "  time float = 1 s"
    )
    return dip.parse()


def test_load(env, tmp_path):
    file = tmp_path / "environment.diph5"
    env.save(file)

    loaded = Environment()
    loaded.load(file)

    assert loaded.size == 7
    assert loaded["title"].value == "Python environment"
    assert loaded["simulation.steps"].value == 100
    assert loaded["simulation.timestep"].value == 0.5
    assert len(loaded["boundary"].items()) == 1
    assert len(loaded["samples"].elements()) == 2


def test_save(env, tmp_path):
    file = tmp_path / "environment.diph5"

    env.save(file)

    assert file.is_file()
    assert file.stat().st_size > 0


def test_generate(env, tmp_path):
    assert env.size == 7

    with pytest.raises(RuntimeError, match="Generating a static parameter list is not implemented yet"):
        env.generate(OutputFormat.JSON, tmp_path / "parameters.json")
