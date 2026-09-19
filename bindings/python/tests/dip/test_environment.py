import pytest

from scinumtools3.dip import DIP, Environment, ExportFormat


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


def test_source_manifest_and_provenance(tmp_path):
    parser = DIP()
    parser.add_string("value int = 1\n")
    environment = parser.parse()

    provenance = environment["value"].provenance
    assert provenance.source_name
    assert provenance.source_line == 1
    assert provenance.source_code == "value int = 1"
    assert provenance.source is not None
    assert provenance.source.name == provenance.source_name
    assert provenance.source.hash_algorithm == "SHA-256"
    assert provenance.source.hash == "cf4c47b9b0b584c2bfe69a84ca55d3b34513d9b9a07594e9aa879e555c1ee9ef"
    assert any(source.name == provenance.source_name for source in environment.source_manifest)

    file = tmp_path / "environment.diph5"
    environment.save(file)
    loaded = Environment()
    loaded.load(file)
    assert loaded["value"].provenance.source.hash == provenance.source.hash


def test_generate(env, tmp_path):
    assert env.size == 7

    file = tmp_path / "parameters.json"
    julia_file = tmp_path / "parameters.jl"
    env.generate(ExportFormat.JSON, file)
    env.generate(ExportFormat.JULIA, julia_file)

    assert file.is_file()
    assert '"simulation"' in file.read_text()
    assert '"inlet"' in file.read_text()
    assert "const parameters" in julia_file.read_text()
