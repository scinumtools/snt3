import numpy as np
import pytest

from scinumtools3.core import DataType

from scinumtools3.dip import (
    ArtifactKind,
    DIP,
    DiagnosticSeverity,
    DependencyEventKind,
    Environment,
    ValueChangeKind,
    detect_artifact,
    diagnostic_from_exception,
    inspect_capabilities,
    inspect_dependency_graph,
    inspect_table,
    inspect_tables,
    inspect_value,
    inspect_values,
    open_artifact,
    read_value_slice,
    reload_artifact,
)


def test_dependency_graph_exposes_operation_tree_and_survives_snapshot(tmp_path):
    fast_parser = DIP()
    fast_parser.add_string("distance float = 12 m\ntime float = 3 s\nspeed float = ({?distance} / {?time}) m/s\n")
    fast_env = fast_parser.parse()
    assert not inspect_dependency_graph(fast_env).recorded
    assert inspect_dependency_graph(fast_env).events == []

    parser = DIP()
    parser.add_string("distance float = 12 m\ntime float = 3 s\nspeed float = ({?distance} / {?time}) m/s\n")
    env = parser.parse(record_dependency_graph=True)

    def check(current):
        graph = inspect_dependency_graph(current)
        assert graph.recorded
        event = graph.latest("?speed", DependencyEventKind.Value)
        assert event is not None
        assert {edge.target for edge in graph.dependencies("?speed")} == {"?distance", "?time"}
        assert all(edge.operand for edge in graph.dependencies("?speed"))
        assert event.composition is not None
        assert len(event.composition.nodes) >= 3
        assert graph.referenced_by("?distance") == ["?speed"]
        assert inspect_capabilities(current, "speed").has_reference_graph

    check(env)
    snapshot = tmp_path / "graph.diph5"
    env.save(snapshot)
    loaded = Environment()
    loaded.load(snapshot)
    check(loaded)


def test_table_order_and_column_values_survive_diph5(tmp_path):
    parser = DIP()
    parser.add_string(
        'measurements table = """zeta float m/s\nalpha int\n---\n2.5 7\n3.5 8\n"""\n'
    )
    env = parser.parse()

    def check_table(current):
        table = inspect_table(current, "measurements")
        assert table.path == "measurements"
        assert table.rows == 2
        assert [(column.index, column.name, column.path) for column in table.columns] == [
            (0, "zeta", "measurements.zeta"),
            (1, "alpha", "measurements.alpha"),
        ]
        assert table.columns[0].units is not None
        assert table.columns[1].units is None
        with pytest.raises(AttributeError):
            table.rows = 3
        with pytest.raises(AttributeError):
            table.columns[0].name = "changed"
        assert [item.path for item in inspect_tables(current)] == ["measurements"]
        assert inspect_value(current, table.columns[0].path).value == [2.5, 3.5]
        assert inspect_value(current, table.columns[1].path).value == [7, 8]
        assert read_value_slice(current, table.columns[0].path, [(1, 1)]) == 3.5
        assert inspect_capabilities(current, "measurements").has_tabular_data

    check_table(env)
    snapshot = tmp_path / "measurements.diph5"
    env.save(snapshot)
    assert detect_artifact(snapshot) == ArtifactKind.DIPH5

    loaded = Environment()
    loaded.load(snapshot)
    check_table(loaded)
    check_table(open_artifact(snapshot))


def test_value_inspection_changes_capabilities_and_diagnostics(tmp_path):
    parser = DIP()
    parser.add_string("physics\n  speed float = 2 m/s\n    ?descr \"Flow speed\"\n")
    parser.add_override_string("physics.speed = 3 m/s\n")
    env = parser.parse()

    inspected = inspect_value(env, "physics.speed")
    assert inspected.path == "physics.speed"
    assert inspected.value == 3.0
    assert inspected.units is not None
    assert inspected.metadata.description == "Flow speed"
    assert inspected.declaration_location.line == 2
    assert inspected.override_location.line == 1
    assert [change.kind for change in inspected.changes] == [
        ValueChangeKind.Declaration,
        ValueChangeKind.Override,
    ]
    assert [value.path for value in inspect_values(env)] == ["physics.speed"]
    assert inspect_capabilities(env, "physics").has_children
    assert not inspect_capabilities(env, "physics").has_value

    snapshot = tmp_path / "physics.diph5"
    env.save(snapshot)
    reload_artifact(env, snapshot)
    assert inspect_value(env, "physics.speed").value == 3.0

    with pytest.raises(RuntimeError) as caught:
        invalid = DIP()
        invalid.add_string("broken int = 1\n")
        invalid.add_override_string("missing = 2\n")
        invalid.parse()
    diagnostic = diagnostic_from_exception(caught.value)
    assert diagnostic is not None
    assert diagnostic.severity == DiagnosticSeverity.Error
    assert diagnostic.code.startswith("dip.")
    assert diagnostic.message
    assert diagnostic.location is not None
    assert diagnostic.location.line == 1


def test_python_inspection_preserves_integer_precision_and_array_shape(tmp_path):
    parser = DIP()
    parser.add_string(
        "large uint64 = 9007199254740993\n"
        "maximum uint64 = 18446744073709551615\n"
        "minimum int64 = -9223372036854775808\n"
        "counts uint64[1] = [18446744073709551615]\n"
        "offsets int64[1] = [-9223372036854775808]\n"
        'labels str[1] = ["only"]\n'
        "grid uint64[2,2] = [[0, 9007199254740993], [18446744073709551615, 7]]\n"
    )
    env = parser.parse()

    def check_values(current):
        assert inspect_value(current, "large").value == 2**53 + 1
        assert inspect_value(current, "maximum").value == 2**64 - 1
        assert inspect_value(current, "minimum").value == -(2**63)
        for path, dtype, expected in (
            ("counts", DataType.Integer64_U, 2**64 - 1),
            ("offsets", DataType.Integer64, -(2**63)),
            ("labels", DataType.String, "only"),
        ):
            inspected = inspect_value(current, path)
            assert inspected.type == dtype
            assert inspected.shape == [1]
            values = inspected.to_numpy()
            assert values.shape == (1,)
            if dtype == DataType.Integer64_U:
                assert values.dtype == np.dtype("uint64")
            elif dtype == DataType.Integer64:
                assert values.dtype == np.dtype("int64")
            assert values[0] == expected
        grid = inspect_value(current, "grid")
        assert grid.shape == [2, 2]
        assert grid.value == [[0, 2**53 + 1], [2**64 - 1, 7]]
        np.testing.assert_array_equal(grid.to_numpy(), np.array(grid.value, dtype=np.uint64))

    check_values(env)
    snapshot = tmp_path / "numeric.diph5"
    env.save(snapshot)
    loaded = Environment()
    loaded.load(snapshot)
    check_values(loaded)
