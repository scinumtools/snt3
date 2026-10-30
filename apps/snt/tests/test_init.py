from conftest import checked


def test_init_creates_parseable_project_without_overwriting(cli, tmp_path):
    assert "Command: init" in checked(cli("init", "--help"))
    assert "Examples:" in checked(cli("init", "--help"))
    result = cli("init", cwd=tmp_path)
    output = checked(result)
    assert "DIPfile" in output and "main.dip" in output
    manifest = (tmp_path / "DIPfile").read_text()
    source = (tmp_path / "main.dip").read_text()
    assert manifest == 'code[]\n  file = "main.dip"\n'
    assert source == "# Replace this example with your project parameters.\nexample int = 1\n"
    assert "example" in checked(cli("dip", "parse", "--project", "DIPfile", "--print", cwd=tmp_path))

    repeated = cli("init", cwd=tmp_path)
    assert repeated.returncode != 0
    assert b"already exists" in repeated.stderr
    assert (tmp_path / "DIPfile").read_text() == manifest
    assert (tmp_path / "main.dip").read_text() == source

    collision = tmp_path / "collision"
    collision.mkdir()
    (collision / "main.dip").write_text("existing source\n")
    assert cli("init", cwd=collision).returncode != 0
    assert not (collision / "DIPfile").exists()
    assert (collision / "main.dip").read_text() == "existing source\n"
