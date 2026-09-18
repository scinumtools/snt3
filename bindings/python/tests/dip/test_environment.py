import pytest

from scinumtools3.dip import DIP, OutputFormat


@pytest.fixture
def env():
    dip = DIP()
    dip.add_string(
        'title str = "Python environment"\n'
        "simulation\n"
        "  steps int = 100\n"
        "  timestep float = 0.5 fs\n"
        "  enabled bool = true"
    )
    return dip.parse()


def test_load(env, tmp_path):
    assert env.size == 4

    with pytest.raises(RuntimeError, match="Loading an environment from HDF5 is not implemented yet"):
        env.load(tmp_path / "environment.h5")


def test_save(env, tmp_path):
    assert env.size == 4

    with pytest.raises(RuntimeError, match="Saving an environment to HDF5 is not implemented yet"):
        env.save(tmp_path / "environment.h5")


def test_generate(env, tmp_path):
    assert env.size == 4

    with pytest.raises(RuntimeError, match="Generating a static parameter list is not implemented yet"):
        env.generate(OutputFormat.JSON, tmp_path / "parameters.json")
