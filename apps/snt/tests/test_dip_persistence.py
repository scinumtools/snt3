from conftest import checked


def test_diph5_save_load_and_arguments(cli, tmp_path):
    saved = tmp_path / "parameters with spaces.diph5"
    copied = tmp_path / "copy.diph5"

    def parse(*args, output=b""):
        result = cli("dip", "parse", *args)
        checked(result)
        assert result.stdout == output
        return result

    def fails(*args):
        result = cli("dip", "parse", *args)
        assert result.returncode != 0 and result.stderr
        assert b"Saved DIPH5:" not in result.stderr

    parse("--input", "string", 'steps int = 100\nenabled bool = true\n  !tags ["selected"]\n', "--save", saved)
    assert saved.exists()
    parse("--load", saved, "--request", "steps", "--value", "--type", "integer", output=b"100\n")
    result = parse("--load", saved, "--tags", "selected", "--print", "--save", copied, output=b"enabled = true\n")
    assert result.stderr.decode().replace("\\", "/").replace("\r\n", "\n") == f"Saved DIPH5: {copied}\n"
    parse("--load", copied, "--request", "steps", "--value", "--type", "integer", output=b"100\n")
    parse("--load", copied, "--save", copied)
    parse("--load", copied, "--request", "steps", "--value", output=b"100\n")

    fails("--input", "string", "steps int = 2", "--request", "steps", "--value", "--type", "bool", "--save", saved)
    parse("--load", saved, "--request", "steps", "--value", output=b"100\n")
    for args in [
        ("--load", saved, "--input", "string", "other int = 1"),
        ("--input", "string", "other int = 1", "--load", saved),
        ("--load",), ("--load", saved, copied), ("--load", saved, "--load", saved),
        ("--load", saved, "--save"), ("--load", saved, "--save", copied, saved),
        ("--load", saved, "--save", copied, "--save", copied), ("--save", saved),
        ("--load", tmp_path / "missing.diph5"),
        ("--load", saved, "--save", saved / "invalid.diph5"),
    ]:
        fails(*args)

    parse("--input", "string", "steps int = 2", "--save", saved)
    parse("--load", saved, "--request", "steps", "--value", output=b"2\n")

    project = tmp_path / "project"
    project.mkdir()
    (project / "parameters.dip").write_text("answer int = 42\n")
    (project / "DIPfile").write_text('code[]\n  file = "parameters.dip"\n')
    manifest = project / "DIPfile"
    parse("--project", manifest, "--request", "answer", "--value", "--type", "integer", output=b"42\n")
    snapshot = project / "project.diph5"
    parse("--project", manifest, "--record-graph", "--relative-source-paths", "--save", snapshot)
    parse("--load", snapshot, "--request", "answer", "--value", "--type", "integer", output=b"42\n")
    fails("--load", snapshot, "--record-graph")
    fails("--project", manifest, "--relative-source-paths")
    fails("--project", manifest, "--input", "string", "other int = 1")
    for args in [("--project", manifest, "--input", "override_string", "answer = 99"),
                 ("--input", "override_string", "answer = 99", "--project", manifest)]:
        parse(*args, "--request", "answer", "--value", "--type", "integer", output=b"99\n")
    schema = tmp_path / "schema.dipl"
    schema.write_text("value int = 42\n")
    parse("-i", "schema_file", "settings", schema, "-i", "string", "physics : settings", "-r", "physics.value", "--value", "--type", "integer", output=b"42\n")
    parse("-i", "schema_string", "settings", "value int = 42", "-i", "string", "physics : settings", "-r", "physics.value", "--value", output=b"42\n")
    fails("-i", "schema_string", "settings")
    override = project / "overrides.dip"
    override.write_text("answer = 99\n")
    parse("--input", "override_file", override, "--project", manifest, "--request", "answer", "--value", "--type", "integer", output=b"99\n")
