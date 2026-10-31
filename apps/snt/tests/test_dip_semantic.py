import json
from pathlib import Path

from conftest import checked, fixture_path


def test_semantic_json_contract(cli, source_root, tmp_path):
    input_file = source_root / "tests/dip/fixtures/generated_parameters.dip"
    graph_input = source_root / "examples/dip/SemanticDescription/parameters.dip"
    fixture = Path(__file__).with_name("dip_semantic_expected.json").read_text()
    expected = json.loads(fixture.replace("@INPUT@", fixture_path(input_file))
                          .replace("@GRAPH_INPUT@", fixture_path(graph_input)))

    def query(*args):
        return json.loads(checked(cli("dip", *args)))

    assert query("describe", "--input", input_file, "--path", "experiment.steps", "--format", "json") == expected["describe"]
    assert query("override-contract", "--input", input_file, "--path", "experiment.steps", "--format", "json") == expected["contract"]
    assert query("describe", "--input", graph_input, "--path", "simulation.speed", "--record-graph", "--format", "json") == expected["graph_describe"]
    assert query("list", "--input", input_file, "--query", "?experiment.", "--limit", "1", "--format", "json") == {
        "schema_version": "1", "total": 4, "truncated": True, "items": [expected["list_item"]]
    }
    assert query("preview", "--input", input_file, "--override", "experiment.steps = 200", "--format", "json") == expected["valid_preview"]
    assert query("preview", "--input", input_file, "--override", "missing = 2", "--format", "json") == expected["invalid_preview"]

    source = tmp_path / "source-metadata.dip"
    source.write_text('?authors "Sample authors"\n?title ""\nconcept str = "sample"\n')
    sources = query("sources", "--input", source, "--format", "json")["sources"]
    assert any(item["metadata"]["authors"] == "Sample authors" and
               item["metadata"]["title"] == "" and
               "title" in item["metadata"]["present"] for item in sources)

    schema_metadata = tmp_path / "schema-metadata.dip"
    schema_metadata.write_text('$schema evidence\n  source_id str\n    ?category "Citation"\n    ?rationale "Schema rationale"\nrecords map : evidence\nrecords[first]\n  source_id = "source_a"\n    ?rationale "Local rationale"\n')
    item = query("describe", "--input", schema_metadata, "--path", "records[first].source_id", "--format", "json")
    assert item["metadata"]["rationale"] == "Local rationale"
    assert item["metadata"]["category"] == "Citation"

    metadata_input = tmp_path / "metadata.dip"
    metadata_output = tmp_path / "metadata.json"
    metadata_input.write_text(
        '?authors "Sample team"\n$schema edge\n  source_id str\n    ?rationale "Schema explanation"\n'
        'edges map\n  ?descr "Edge collection"\nedges[uses_material] : edge\n'
        '  ?descr "Material relationship"\n  source_id = "material_1"\n'
        '    ?rationale "Local explanation"\nmeasurements table = """\n'
        '?descr "Measurement table"\ntime float s\n  ?descr "Elapsed time"\n---\n0\n"""\n'
        'empty str = ""\n  ?rationale ""\nabsent str = none\n'
    )
    checked(cli("dip", "parse", "-i", "file", metadata_input, "--generate", "json-metadata", metadata_output))
    data = json.loads(metadata_output.read_text())
    assert data["schema"] == "snt-dip-values/2"
    edge = data["values"]["edges[uses_material].source_id"]
    assert edge["value"] == "material_1"
    assert edge["metadata"]["values"]["rationale"] == "Local explanation"
    nodes = data["node_metadata"]
    assert nodes["edges"]["metadata"]["values"]["description"] == "Edge collection"
    assert nodes["edges[uses_material]"]["kind"] == "map_item"
    assert nodes["edges[uses_material]"]["metadata"]["values"]["description"] == "Material relationship"
    assert nodes["measurements"]["kind"] == "table"
    assert nodes["measurements"]["metadata"]["values"]["description"] == "Measurement table"
    assert nodes["measurements.time"]["kind"] == "table_column"
    assert nodes["measurements.time"]["metadata"]["values"]["description"] == "Elapsed time"
    assert data["values"]["empty"]["value"] == ""
    assert data["values"]["empty"]["presence"] == "value"
    assert data["values"]["empty"]["metadata"]["values"]["rationale"] == ""
    assert "rationale" in data["values"]["empty"]["metadata"]["present"]
    assert data["values"]["absent"]["presence"] == "none"
    assert data["values"]["absent"]["value"] is None
    assert any(item["metadata"]["values"].get("authors") == "Sample team" for item in data["sources"].values())


def test_schema_hierarchy_live_and_snapshot(cli, source_root, tmp_path):
    source = source_root / "tests/dip/fixtures/schema_hierarchy.dip"
    live = json.loads(checked(cli("dip", "schemas", "--input", source, "--format", "json")))
    assert live["schema"] == "snt-schema-hierarchy/2"
    assert live["definitions_available"] and live["applications_complete"]
    assert len(live["definitions"]) == 4 and len(live["applications"]) == 6
    assert live["definitions"][1]["members"][1]["schema_refs"][0] == "address"
    assert live["definitions"][0]["origin"]["file"] == "schema_hierarchy.dip"
    assert live["applications"][1]["path"] == "people[alice]"
    assert live["applications"][1]["inherited_from_collection"]
    assert live["applications"][2]["schema_ids"][0] == "role"
    assert live["values"][0]["contributing_schema_id"] == "person"

    snapshot = tmp_path / "schema.diph5"
    checked(cli("dip", "parse", "-i", "file", source, "--save", snapshot))
    saved = json.loads(checked(cli("dip", "schemas", "--input", snapshot, "--format", "json")))
    assert saved["definitions_available"] is False and saved["applications_complete"] is False
    assert saved["definitions"] == []
    assert saved["applications"][1]["inherited_from_collection"] is None
    assert saved["applications"][1]["schema_ids"][0] == "person"
