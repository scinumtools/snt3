import json

from conftest import checked, normalize_path


def test_validate_cli(cli, tmp_path):
    valid = tmp_path / "valid.dip"
    invalid = tmp_path / "invalid.dip"
    multiple = tmp_path / "multiple.dip"
    manifest = tmp_path / "DIPfile"
    renamed = tmp_path / "experiment.manifest"
    invalid_manifest = tmp_path / "invalid.manifest"
    valid.write_text("answer int = 42\n")
    invalid.write_text("answer unknown_type = 42\n")
    multiple.write_text("first unknown_type = 1\nvalid int = 2\nsecond unknown_type = 3\n")
    manifest.write_text('code[]\n  file = "valid.dip"\n')
    renamed.write_text(manifest.read_text())
    invalid_manifest.write_text("first unknown_type = 1\nsecond unknown_type = 2\n")
    settings = tmp_path / "settings"
    settings.mkdir()
    (settings / "DIPfile").write_text('validate\n  max_syntax_errors = 1\ncode[]\n  file = "limited.dip"\n')
    (settings / "limited.dip").write_text("first unknown_type = 1\nsecond unknown_type = 2\n")
    bad_settings = tmp_path / "invalid-settings"
    bad_settings.mkdir()
    (bad_settings / "DIPfile").write_text("validate\n  max_syntax_errors = 0\n")

    assert "valid" in checked(cli("validate", valid))
    assert "error [dip." in cli("validate", invalid).stderr.decode()
    checked(cli("validate", manifest))
    checked(cli("validate", "--project", renamed))

    def validate_json(*args):
        return json.loads(checked(cli("validate", "--json", *args), 1))

    result = validate_json(invalid)
    assert result["valid"] is False and result["diagnostics"][0]["code"].startswith("dip.")
    result = validate_json(multiple)
    assert [item["location"]["line"] for item in result["diagnostics"]] == [1, 3]
    manifest.write_text('code[]\n  file = "multiple.dip"\n')
    result = validate_json(manifest)
    assert len(result["diagnostics"]) == 2
    assert normalize_path(result["diagnostics"][0]["location"]["source"]) == normalize_path(multiple)
    assert len(validate_json("--project", invalid_manifest)["diagnostics"]) == 2
    for path in (settings / "limited.dip", settings / "DIPfile"):
        assert len(validate_json(path)["diagnostics"]) == 1
    result = validate_json(bad_settings / "DIPfile")
    assert result["diagnostics"][0]["location"]["line"] == 2
    assert normalize_path(result["diagnostics"][0]["location"]["source"]) == normalize_path(bad_settings / "DIPfile")
    assert "Invalid validation setting" in str(result)
    combined = cli("validate", valid, invalid)
    checked(combined, 1)
    assert b"valid" in combined.stdout and b"invalid" in combined.stderr
    assert [item["valid"] for item in json.loads(checked(cli("validate", "--json", valid, invalid), 1))] == [True, False]
    missing = cli("validate", tmp_path / "missing.dip")
    checked(missing, 2)
    assert b"dip.io" in missing.stderr
