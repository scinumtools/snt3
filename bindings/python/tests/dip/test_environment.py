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


def test_extended_metadata():
    parser = DIP()
    parser.add_string(
        "gravity.force_accuracy.opening_angle float = 0.7\n"
        "  ?descr \"Barnes-Hut opening angle\"\n"
        "  ?native \"ErrTolTheta\"\n"
        "  ?requires [\"build.gravity.self_gravity\", \"build.gravity.tree\"]\n"
        "  ?conflicts \"build.gravity.direct\"\n"
        "  ?implies \"build.gravity.tree\"\n"
        "  ?see [\"gravity.force_accuracy\", \"gravity.softening\"]\n"
        "  ?example [\"0.5\", \"0.7\"]\n"
        "  ?rationale \"Balances accuracy and cost\"\n"
        "  ?recommended_range \"0.3 to 1.0\"\n"
        "  ?performance_impact \"Smaller values cost more\"\n"
        "  ?scientific_impact \"Smaller values improve accuracy\"\n"
        "  ?category \"Gravity\"\n"
        "  ?visibility \"advanced\"\n"
    )
    environment = parser.parse()
    metadata = environment["gravity.force_accuracy.opening_angle"].metadata

    assert metadata.description == "Barnes-Hut opening angle"
    assert metadata.native == ["ErrTolTheta"]
    assert metadata.requires == ["build.gravity.self_gravity", "build.gravity.tree"]
    assert metadata.conflicts == ["build.gravity.direct"]
    assert metadata.implies == ["build.gravity.tree"]
    assert metadata.see == ["gravity.force_accuracy", "gravity.softening"]
    assert metadata.example == ["0.5", "0.7"]
    assert metadata.rationale == "Balances accuracy and cost"
    assert metadata.recommended_range == "0.3 to 1.0"
    assert metadata.performance_impact == "Smaller values cost more"
    assert metadata.scientific_impact == "Smaller values improve accuracy"
    assert metadata.category == "Gravity"
    assert metadata.visibility == "advanced"


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
