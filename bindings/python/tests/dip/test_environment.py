import pytest

from scinumtools3.dip import Environment, OutputFormat


def test_load(tmp_path):
    env = Environment()

    with pytest.raises(RuntimeError, match="Loading an environment from HDF5 is not implemented yet"):
        env.load(tmp_path / "environment.h5")


def test_save(tmp_path):
    env = Environment()

    with pytest.raises(RuntimeError, match="Saving an environment to HDF5 is not implemented yet"):
        env.save(tmp_path / "environment.h5")


def test_generate(tmp_path):
    env = Environment()

    with pytest.raises(RuntimeError, match="Generating a static parameter list is not implemented yet"):
        env.generate(OutputFormat.JSON, tmp_path / "parameters.json")
