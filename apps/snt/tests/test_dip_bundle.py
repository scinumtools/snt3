import json

from conftest import checked


def test_project_bundle(cli, tmp_path):
    (tmp_path / "DIPfile").write_text(
        'sources[]\n  name = "data"\n  filepath = "data.dipt"\n'
        'code[]\n  file = "model.dip"\n'
    )
    (tmp_path / "data.dipt").write_text("mass float kg\n---\n3\n")
    (tmp_path / "model.dip").write_text("samples table = {data}\nscale float = 2\n")
    bundle = tmp_path / "project.bundle.json"
    checked(cli("dip", "package", "--project", tmp_path / "DIPfile", "--output", bundle))
    data = json.loads(bundle.read_text())
    assert data["format"] == "snt-dipl-project-bundle/1"
    assert data["entry"] == "DIPfile"
    assert len(data["files"]) == 3
