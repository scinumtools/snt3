import gc

import pytest

from scinumtools3.dip import DIP, Environment
from scinumtools3.puq import Quantity


@pytest.fixture
def selection_env():
    parser = DIP()
    parser.add_string(
        'physics int = 1\n'
        '  !tags ["export", "runtime"]\n'
        '  speed float = 2 m/s\n'
        '    !tags ["export", "runtime", "hydro"]\n'
        '    ?descr "Flow speed"\n'
        '  gravity int = 3\n'
        '    !tags ["export", "gravity", "internal"]\n'
        '  plain int = 4\n'
        '  samples[]\n'
        '    value int = 5\n'
        '  samples[]\n'
        '    value int = 6\n'
        '  boundaries[inlet]\n'
        '    value int = 7\n'
        'physics_extra int = 8\n'
    )
    return parser.parse()


def names(nodes):
    return [node.name for node in nodes]


@pytest.mark.parametrize("filters, expected", [
    ({"tags_all": ["export", "runtime"]}, ["physics", "physics.speed"]),
    ({"tags_any": ["hydro", "gravity"]}, ["physics.speed", "physics.gravity"]),
    ({"tags_none": ["export"]}, ["physics.plain", "physics.samples[0].value",
                                "physics.samples[1].value", "physics.boundaries[inlet].value", "physics_extra"]),
    ({"tags_all": ["export", "runtime"], "tags_any": ["hydro", "gravity"],
      "tags_none": ["internal", "deprecated"]}, ["physics.speed"]),
    ({"tags_all": ["missing"]}, []),
    ({"tags_any": ["missing"]}, []),
    ({"tags_all": ["export"], "tags_none": ["export"]}, []),
    ({"tags_any": ["export", "export", "hydro"]}, ["physics", "physics.speed", "physics.gravity"]),
])
def test_tag_filters(selection_env, filters, expected):
    # Filters combine with AND, use explicit tags, and never duplicate results.
    assert names(selection_env.select(**filters)) == expected


def test_scopes_and_order(selection_env):
    expected = ["physics", "physics.speed", "physics.gravity", "physics.plain",
                "physics.samples[0].value", "physics.samples[1].value",
                "physics.boundaries[inlet].value", "physics_extra"]
    assert names(selection_env.select()) == expected
    assert names(selection_env.select("?", tags_all=[], tags_any=[], tags_none=[])) == expected
    # A subtree includes its value-bearing root, but not similarly named siblings.
    assert names(selection_env.select("?physics.")) == expected[:-1]
    assert names(selection_env.select("?physics")) == ["physics"]
    assert names(selection_env.select("?physics.speed")) == ["physics.speed"]
    assert names(selection_env.select("?physics.samples.")) == expected[4:6]
    assert names(selection_env.select("?physics.samples[0].")) == expected[4:5]
    assert names(selection_env.select("?physics.boundaries.")) == expected[6:7]
    assert selection_env.select("?missing.") == []
    with pytest.raises(RuntimeError, match="Invalid node request"):
        selection_env.select("physics")
    # Existing requests still return relative paths and match any supplied tag.
    assert names(selection_env.request_group("?physics.", ["hydro", "gravity"])) == ["speed", "gravity"]
    assert names(selection_env.request_group("?physics.speed")) == ["speed"]
    with pytest.raises(RuntimeError, match="Node request returns empty node group"):
        selection_env.request_group("?", ["missing"])


def test_selected_node_access(selection_env):
    node = selection_env.select("?physics.speed")[0]
    assert node.value == 2
    assert node.units == Quantity("m/s")
    assert node.tags == ["export", "runtime", "hydro"]
    tags = node.tags
    tags.clear()
    assert selection_env.select("?physics.speed")[0].tags == ["export", "runtime", "hydro"]
    metadata = node.metadata
    del node, selection_env
    gc.collect()
    assert metadata.description == "Flow speed"


def test_schema_override_and_round_trip(tmp_path):
    parser = DIP()
    parser.add_string(
        '$schema settings\n'
        '  speed float = 1 m/s\n'
        '    !tags ["export", "runtime"]\n'
        '    ?descr "Configured speed"\n'
        '    ?native ["Speed", "SPEED"]\n'
        'physics : settings\n'
        '  speed = 3\n'
    )
    env = parser.parse()
    file = tmp_path / "selection.diph5"
    env.save(file)
    loaded = Environment()
    loaded.load(file)
    # Schema tags and metadata survive a value override and persistence.
    for source in (env, loaded):
        nodes = source.select(tags_all=["export", "runtime"])
        assert names(nodes) == ["physics.speed"]
        node = nodes[0]
        assert node.value == 3
        assert node.units == Quantity("m/s")
        assert node.metadata.description == "Configured speed"
        assert node.metadata.native == ["Speed", "SPEED"]


def test_source_qualified_selection(tmp_path):
    source = tmp_path / "source.dip"
    source.write_text('physics.speed int = 2\n  !tags ["export"]\n')
    parser = DIP()
    parser.add_string(f'$source model = "{source}"\n')
    env = parser.parse()
    assert env.select() == []
    for query in ("model?", "model?physics.", "model?physics.speed"):
        assert names(env.select(query, tags_all=["export"])) == ["physics.speed"]
