from conftest import checked


def test_saved_environment_comparison(cli, tmp_path):
    before, after = tmp_path / "before.diph5", tmp_path / "after.diph5"
    checked(cli("dip", "parse", "--input", "string", "value int = 1", "--save", before))
    checked(cli("dip", "parse", "--input", "string", "value int = 2", "--save", after))
    assert "0 added, 0 removed, 0 changed" in checked(cli("dip", "compare", before, before))
    different = checked(cli("dip", "compare", before, after, "--max-details", "0"), 1)
    assert "1 changed" in different and "1 more differences" in different
    invalid = cli("dip", "compare", before, tmp_path / "missing.diph5")
    assert invalid.returncode == 2 and invalid.stderr
