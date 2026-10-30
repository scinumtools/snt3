import json

from conftest import checked


def test_lint_cli(cli, tmp_path):
    clean = tmp_path / "clean.dip"
    advisory = tmp_path / "advisory.dip"
    invalid = tmp_path / "invalid.dip"
    choices = tmp_path / "choices.dip"
    manifest = tmp_path / "DIPfile"
    clean.write_text("steps int = 8\n")
    advisory.write_text('steps int = 8\n  ?visibility "public"\nold int = 3\n  ?deprecated "Use steps"\n')
    invalid.write_text("bad unknown_type = 1\nother unknown_type = 2\n")
    choices.write_text('level int = 1\n  ?visibility "public"\n  !options [1, 1, 2]\n')
    manifest.write_text('lint\n  require_public_descriptions = false\n  warn_deprecated_values = true\ncode[]\n  file = "advisory.dip"\n')
    optin = tmp_path / "optin"
    optin.mkdir()
    (optin / "DIPfile").write_text('lint\n  warn_broken_metadata_references = true\n  warn_unused_schemas = true\ncode[]\n  file = "optional.dip"\n')
    (optin / "optional.dip").write_text('$schema spare\n  value int = 1\nroot\n  child int = 1\ncurrent int = 2\n  ?requires "root.missing"\n')
    assert "clean" in checked(cli("lint", clean))

    def lint_json(path):
        return json.loads(checked(cli("lint", "--json", path), 1))

    result = lint_json(advisory)
    assert result["valid"] is True and result["clean"] is False
    assert len(result["diagnostics"]) == 1
    assert result["diagnostics"][0]["code"] == "dip.lint.deprecated-value"
    assert result["diagnostics"][0]["location"]["line"] == 3
    assert lint_json(manifest)["diagnostics"][0]["code"] == "dip.lint.deprecated-value"
    result = lint_json(invalid)
    assert result["valid"] is False and len(result["diagnostics"]) == 2
    assert [item["code"] for item in lint_json(choices)["diagnostics"]] == [
        "dip.lint.duplicate-options", "dip.lint.public-option-labels"
    ]
    assert [item["code"] for item in lint_json(optin / "DIPfile")["diagnostics"]] == [
        "dip.lint.broken-metadata-reference", "dip.lint.unused-schema"
    ]
