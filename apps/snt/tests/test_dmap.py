from conftest import checked


def test_dimension_map_generation(cli, tmp_path):
    directory = tmp_path / "src/snt/puq/systems/dmaps"
    directory.mkdir(parents=True)
    assert "Generating empty dmap file" in checked(cli("dmap", "-e", cwd=tmp_path))
    maps = list(directory.glob("dmap_*.h"))
    assert maps
    assert all(path.read_text() == "// Empty file" for path in maps)
    assert "Generating dmap file" in checked(cli("dmap", cwd=tmp_path))
    assert all("Unit system:" in path.read_text() and "Code version:" in path.read_text() for path in maps)
