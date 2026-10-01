#include "pch_tests.h"

#include <snt/dip/dip.h>
#include <snt/dip/diagnostic.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/inspection.h>
#include <snt/val/values_array.h>

#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>

using namespace snt;

TEST(Inspection, EvaluatedValuesAndProvenance) {
    dip::DIP parser;
    parser.add_schema_string("settings", "speed float = 2 m/s\n  ?descr \"Flow speed\"\n");
    parser.add_string("physics : settings\ncount int = 4\n");
    parser.add_override_string("physics.speed = 3 m/s\n");
    const auto env = parser.parse();

    auto speed = dip::inspect_value(env, "physics.speed");
    ASSERT_NE(speed.value, nullptr);
    EXPECT_EQ(speed.path, "physics.speed");
    EXPECT_EQ(speed.metadata.description, "Flow speed");
    EXPECT_TRUE(speed.units.has_value());
    EXPECT_TRUE(speed.override_location.has_value());
    ASSERT_EQ(speed.changes.size(), 2);
    EXPECT_EQ(speed.changes[0].kind, dip::ValueChangeKind::Declaration);
    EXPECT_EQ(speed.changes[1].kind, dip::ValueChangeKind::Override);
    EXPECT_EQ(speed.override_location->line, 1);
    EXPECT_FALSE(speed.declaration_location.source.empty());
    EXPECT_TRUE(speed.contributing_schema.has_value());
    EXPECT_EQ(speed.contributing_schema->name, "settings");
    EXPECT_EQ(env["physics.speed"].as<double>(), 3);

    const auto values = dip::inspect_values(env);
    ASSERT_EQ(values.size(), 2);
    EXPECT_EQ(values[0].path, "physics.speed");
    EXPECT_EQ(values[1].path, "count");
    EXPECT_EQ(values[1].value->get_dtype(), values[1].type);
    EXPECT_FALSE(values[1].override_location.has_value());
}

TEST(Inspection, BranchCollectionsUseSemanticPaths) {
    dip::DIP parser;
    parser.add_string(
        "enabled bool = true\n"
        "@if ({?enabled} == true)\n"
        "  status str = \"fast\"\n"
        "  group\n"
        "    member int = 4\n"
        "@end\n"
        "@if false\n"
        "  hidden int = 1\n"
        "@end\n"
    );
    const auto env = parser.parse(true);
    EXPECT_TRUE(env.hierarchy.has_collection("status"));
    EXPECT_TRUE(env.hierarchy.has_collection("group"));
    EXPECT_TRUE(env.hierarchy.has_collection("group.member"));
    EXPECT_FALSE(env.hierarchy.has_collection("hidden"));
    for (const auto& entry : env.hierarchy.get_collections())
        EXPECT_FALSE(env.branching.is_internal_path(entry.first)) << entry.first;
    for (const auto& node : env.nodes.get_nodes())
        EXPECT_TRUE(env.hierarchy.has_collection(node->path.name)) << node->path.name;
    EXPECT_EQ(env["status"].get_provenance().source_line, 3);
    const auto status = dip::inspect_value(env, "status");
    EXPECT_EQ(status.value->to_string(), "\"fast\"");
    EXPECT_EQ(status.declaration_location.line, 3);
}

TEST(Inspection, BranchCollectionsRetainMembers) {
    dip::DIP parser;
    parser.add_string(
        "@if true\n"
        "  items[]\n"
        "    value int = 1\n"
        "  lookup[key]\n"
        "    value int = 2\n"
        "@end\n"
    );
    const auto env = parser.parse();
    EXPECT_EQ(env.hierarchy.get_collection("items").items, std::vector<std::string>({"0"}));
    EXPECT_TRUE(env.hierarchy.has_collection("items[0]"));
    EXPECT_TRUE(env.hierarchy.has_collection("items[0].value"));
    EXPECT_EQ(env.hierarchy.get_collection("lookup").items, std::vector<std::string>({"key"}));
    EXPECT_TRUE(env.hierarchy.has_collection("lookup[key]"));
    EXPECT_TRUE(env.hierarchy.has_collection("lookup[key].value"));
}

TEST(Inspection, BranchCollectionsCombineSelectedCases) {
    dip::DIP parser;
    parser.add_string(
        "@if true\n"
        "  items[]\n"
        "    value int = 1\n"
        "  lookup[first]\n"
        "    value int = 2\n"
        "@end\n"
        "@if false\n"
        "  items[]\n"
        "    value int = 99\n"
        "  lookup[skipped]\n"
        "    value int = 99\n"
        "@end\n"
        "@if true\n"
        "  items[]\n"
        "    value int = 3\n"
        "  lookup[second]\n"
        "    value int = 4\n"
        "  @if true\n"
        "    nested int = 5\n"
        "  @end\n"
        "@end\n"
    );
    const auto env = parser.parse();
    EXPECT_EQ(env.hierarchy.get_collection("items").items, std::vector<std::string>({"0", "1"}));
    EXPECT_EQ(env.hierarchy.get_collection("lookup").items, std::vector<std::string>({"first", "second"}));
    for (const auto& path : {"items[0].value", "items[1].value", "lookup[first].value",
                             "lookup[second].value", "nested"}) {
        EXPECT_TRUE(env.hierarchy.has_collection(path)) << path;
        EXPECT_NO_THROW(env[path].get_provenance()) << path;
    }
    EXPECT_FALSE(env.hierarchy.has_collection("items[2]"));
    EXPECT_FALSE(env.hierarchy.has_collection("lookup[skipped]"));
    for (const auto& node : env.nodes.get_nodes())
        EXPECT_TRUE(env.hierarchy.has_collection(node->path.name)) << node->path.name;
    for (const auto& entry : env.hierarchy.get_collections())
        EXPECT_FALSE(env.branching.is_internal_path(entry.first)) << entry.first;
}

TEST(Inspection, NestedBranchKeepsCollectionParent) {
    dip::DIP parser;
    parser.add_string(
        "lookup[key]\n"
        "  @if true\n"
        "    value int = 7\n"
        "  @end\n"
    );
    const auto env = parser.parse();
    EXPECT_EQ(env.hierarchy.get_collection("lookup").items, std::vector<std::string>({"key"}));
    EXPECT_TRUE(env.hierarchy.has_collection("lookup[key].value"));
    EXPECT_NO_THROW(env["lookup[key].value"].get_provenance());
}

TEST(Inspection, CapabilityFlags) {
    dip::DIP parser;
    parser.add_string("physics\n  speed float = 2 m/s\nsamples int[3] = [1, 2, 3]\nsingle int[1] = [4]\n");
    auto env = parser.parse();

    const auto group = dip::inspect_capabilities(env, "physics");
    EXPECT_TRUE(group.hasChildren);
    EXPECT_FALSE(group.hasValue);
    EXPECT_FALSE(group.hasTabularData);

    const auto speed = dip::inspect_capabilities(env, "physics.speed");
    EXPECT_TRUE(speed.hasValue);
    EXPECT_TRUE(speed.hasSource);
    EXPECT_TRUE(speed.hasProvenance);
    EXPECT_FALSE(speed.hasChildren);
    EXPECT_FALSE(speed.hasArrayData);
    EXPECT_FALSE(speed.hasReferenceGraph);
    EXPECT_FALSE(speed.sourceEditable);
    EXPECT_FALSE(speed.directlyWritable);

    EXPECT_TRUE(dip::inspect_capabilities(env, "samples").hasArrayData);
    EXPECT_TRUE(dip::inspect_capabilities(env, "single").hasArrayData);
    EXPECT_THROW(dip::inspect_capabilities(env, "missing"), std::out_of_range);

    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto snapshot = std::filesystem::temp_directory_path() / ("snt-inspection-capabilities-" + suffix + ".diph5");
    env.save(snapshot);
    dip::reload_artifact(env, snapshot);
    EXPECT_FALSE(dip::inspect_capabilities(env, "physics.speed").hasArrayData);
    EXPECT_TRUE(dip::inspect_capabilities(env, "single").hasArrayData);
    std::filesystem::remove(snapshot);
}

TEST(Inspection, DependencyGraphTracksExpressionsAndEffectiveUpdates) {
    dip::DIP parser;
    parser.add_string(
        "source float = 3 m\n"
        "other float = 4 m\n"
        "result float = ({?source} + {?other}) m\n"
        "chosen bool = ({?result} > 5 m && {?source} < 4 m)\n"
        "result = {?source} m\n"
    );
    auto env = parser.parse(true);
    const auto& graph = env.dependency_graph();
    EXPECT_TRUE(graph.recorded);
    const auto* previous = graph.latest("?chosen", dip::DependencyEventKind::Value);
    ASSERT_NE(previous, nullptr);
    ASSERT_TRUE(previous->composition.has_value());
    EXPECT_GT(previous->composition->nodes.size(), 2);
    EXPECT_EQ(previous->reads.front().target, "?result");
    EXPECT_FALSE(previous->reads.front().operand.empty());

    const auto* current = graph.latest("?result", dip::DependencyEventKind::Value);
    ASSERT_NE(current, nullptr);
    ASSERT_EQ(current->reads.size(), 1);
    EXPECT_EQ(current->reads.front().target, "?source");
    EXPECT_EQ(graph.referenced_by("?other").size(), 0);
    EXPECT_EQ(graph.referenced_by("?source").size(), 2);
    EXPECT_TRUE(dip::inspect_capabilities(env, "result").hasReferenceGraph);
    ASSERT_TRUE(env.get_node("result")->units.has_value());

    const auto file = std::filesystem::temp_directory_path() / "snt-dependency-graph-roundtrip.diph5";
    env.save(file);
    dip::Environment restored;
    restored.load(file);
    std::filesystem::remove(file);
    EXPECT_TRUE(restored.dependency_graph().recorded);
    const auto* restored_event = restored.dependency_graph().latest("?chosen", dip::DependencyEventKind::Value);
    ASSERT_NE(restored_event, nullptr);
    ASSERT_TRUE(restored_event->composition.has_value());
    EXPECT_EQ(restored_event->composition->nodes.size(), previous->composition->nodes.size());
    EXPECT_EQ(restored_event->reads.front().operand, previous->reads.front().operand);
    ASSERT_TRUE(restored_event->location.has_value());
    EXPECT_EQ(restored_event->location->line, previous->location->line);
    EXPECT_EQ(restored.dependency_graph().dependencies("?result").front().target, "?source");
}

TEST(Inspection, DependencyRecordingIsOptIn) {
    const std::string code = "base int = 4\nresult int = ({?base} + 2)\n";
    dip::DIP fast_parser;
    fast_parser.add_string(code);
    auto fast = fast_parser.parse();
    EXPECT_FALSE(fast.dependency_graph().recorded);
    EXPECT_TRUE(fast.dependency_graph().events.empty());
    EXPECT_FALSE(dip::inspect_capabilities(fast, "result").hasReferenceGraph);

    dip::DIP graph_parser;
    graph_parser.add_string(code);
    auto recorded = graph_parser.parse(true);
    EXPECT_TRUE(recorded.dependency_graph().recorded);
    ASSERT_EQ(recorded.dependency_graph().dependencies("?result").size(), 1);
    EXPECT_EQ(fast["result"].as<int>(), recorded["result"].as<int>());

    const auto file = std::filesystem::temp_directory_path() / "snt-no-graph-roundtrip.diph5";
    fast.save(file);
    dip::Environment restored;
    restored.load(file);
    std::filesystem::remove(file);
    EXPECT_FALSE(restored.dependency_graph().recorded);
    EXPECT_TRUE(restored.dependency_graph().events.empty());
}

TEST(Inspection, DependencyGraphResolvesRelativeReadsAndConditions) {
    dip::DIP parser;
    parser.add_string(
        "snap int = 30\n"
        "foo\n"
        "  bar\n"
        "    crackle float = 2\n"
        "    result float = ({.crackle} + {...snap})\n"
        "      !condition ({.} > 0)\n"
    );
    const auto env = parser.parse(true);
    const auto& graph = env.dependency_graph();
    const auto reads = graph.dependencies("?foo.bar.result");
    ASSERT_EQ(reads.size(), 2);
    EXPECT_EQ(reads[0].target, "?foo.bar.crackle");
    EXPECT_EQ(reads[1].target, "?snap");
    const auto* condition = graph.latest("?foo.bar.result", dip::DependencyEventKind::Condition);
    ASSERT_NE(condition, nullptr);
    ASSERT_EQ(condition->reads.size(), 1);
    EXPECT_EQ(condition->reads.front().target, "?foo.bar.result");
    EXPECT_TRUE(condition->composition.has_value());
}

TEST(Inspection, DependencyGraphConnectsBranchDecisionsToValues) {
    dip::DIP parser;
    parser.add_string("enabled bool = true\n@if ({?enabled} == true)\n  answer int = 42\n@end\n");
    const auto env = parser.parse(true);
    const auto& graph = env.dependency_graph();
    const auto* decision = graph.latest("#case:1", dip::DependencyEventKind::Decision);
    ASSERT_NE(decision, nullptr);
    ASSERT_EQ(decision->reads.size(), 1);
    EXPECT_EQ(decision->reads.front().target, "?enabled");
    EXPECT_TRUE(decision->composition.has_value());
    const auto* answer = graph.latest("?answer", dip::DependencyEventKind::Value);
    ASSERT_NE(answer, nullptr);
    EXPECT_EQ(answer->controlled_by, std::vector<std::string>({"#case:1"}));
    const auto file = std::filesystem::temp_directory_path() / "snt-branch-graph-roundtrip.diph5";
    env.save(file);
    dip::Environment restored;
    restored.load(file);
    std::filesystem::remove(file);
    const auto* restored_answer = restored.dependency_graph().latest("?answer", dip::DependencyEventKind::Value);
    ASSERT_NE(restored_answer, nullptr);
    EXPECT_EQ(restored_answer->controlled_by, answer->controlled_by);
}

TEST(Inspection, SharedDiagnosticPreservesStructuredException) {
    const dip::Line line{"value int = bad", {"parameters.dip", 12}};
    const dip::SyntaxException error("Invalid value", "Expected integer", "Use an integer",
                                     __FILE__, __LINE__, line);
    const auto diagnostic = dip::diagnostic_from_exception(error);
    EXPECT_EQ(diagnostic.severity, core::DiagnosticSeverity::Error);
    EXPECT_EQ(diagnostic.code, "dip.syntax");
    EXPECT_EQ(diagnostic.message, "Invalid value");
    EXPECT_EQ(diagnostic.details, "Expected integer");
    EXPECT_EQ(diagnostic.suggestion, "Use an integer");
    ASSERT_TRUE(diagnostic.location.has_value());
    EXPECT_EQ(diagnostic.location->source, "parameters.dip");
    EXPECT_EQ(diagnostic.location->line, 12);
    EXPECT_TRUE(diagnostic.origin.has_value());

    const auto generic = dip::diagnostic_from_exception(std::invalid_argument("Invalid artifact"));
    EXPECT_EQ(generic.code, "dip.error");
    EXPECT_EQ(generic.message, "Invalid artifact");
    EXPECT_FALSE(generic.location.has_value());
}

TEST(Inspection, ArtifactDetectionAndFreshReload) {
    EXPECT_EQ(dip::detect_artifact("DIPfile"), dip::ArtifactKind::Project);
    EXPECT_EQ(dip::detect_artifact("model.dip"), dip::ArtifactKind::DIPL);
    EXPECT_EQ(dip::detect_artifact("model.dipl"), dip::ArtifactKind::DIPL);
    EXPECT_EQ(dip::detect_artifact("data.dipt"), dip::ArtifactKind::TableText);
    EXPECT_EQ(dip::detect_artifact("run.diph5"), dip::ArtifactKind::DIPH5);
    EXPECT_EQ(dip::detect_artifact("notes.txt"), dip::ArtifactKind::Unknown);
    EXPECT_THROW(dip::open_artifact("data.dipt"), std::invalid_argument);

    const auto project = std::filesystem::path(PROJECT_SOURCE_ROOT_DIR) / "examples/dip/CreateReport/DIPfile";
    auto project_env = dip::open_artifact(project);
    EXPECT_EQ(project_env["experiment.flow_speed"].as<double>(), 3);

    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto good = std::filesystem::temp_directory_path() / ("snt-inspection-good-" + suffix + ".dip");
    const auto bad = std::filesystem::temp_directory_path() / ("snt-inspection-bad-" + suffix + ".dip");
    const auto snapshot = std::filesystem::temp_directory_path() / ("snt-inspection-run-" + suffix + ".diph5");
    { std::ofstream output(good); output << "answer int = 7\n"; }
    { std::ofstream output(bad); output << "answer int = \"bad\"\n"; }

    auto env = dip::open_artifact(good);
    EXPECT_FALSE(env.dependency_graph().recorded);
    EXPECT_TRUE(dip::open_artifact(good, true).dependency_graph().recorded);
    EXPECT_EQ(env["answer"].as<int>(), 7);
    EXPECT_EQ(dip::inspect_value(env, "answer").declaration_location.source, good.string());
    EXPECT_THROW(dip::reload_artifact(env, bad), std::exception);
    EXPECT_EQ(env["answer"].as<int>(), 7);
    env.save(snapshot);
    dip::reload_artifact(env, snapshot);
    EXPECT_TRUE(env.is_loaded_snapshot());
    EXPECT_EQ(env["answer"].as<int>(), 7);
    auto snapshot_value = dip::inspect_value(env, "answer");
    EXPECT_EQ(snapshot_value.declaration_location.source, good.string());
    EXPECT_EQ(snapshot_value.value->get_dtype(), snapshot_value.type);

    std::filesystem::remove(good);
    std::filesystem::remove(bad);
    std::filesystem::remove(snapshot);
}

TEST(Inspection, BoundedInMemorySlice) {
    dip::DIP parser;
    parser.add_string("samples int[5] = [1, 2, 3, 4, 5]\n");
    const auto env = parser.parse();
    auto slice = dip::read_value_slice(env, "samples", {{1, 3}});
    const auto* integers = dynamic_cast<const val::ArrayValue<int64_t>*>(slice.get());
    ASSERT_NE(integers, nullptr);
    EXPECT_EQ(integers->get_values(), (std::vector<int64_t>{2, 3, 4}));
    EXPECT_THROW(dip::read_value_slice(env, "samples", {{4, 7}}), std::out_of_range);
    EXPECT_THROW(dip::read_value_slice(env, "samples", {}), std::invalid_argument);
}

TEST(Inspection, AppliedModificationHistorySurvivesSnapshot) {
    dip::DIP parser;
    parser.add_string("answer int = 1\nanswer = 2\nanswer = 3\n");
    auto env = parser.parse();
    auto value = dip::inspect_value(env, "answer");
    ASSERT_EQ(value.changes.size(), 3);
    EXPECT_EQ(value.changes[0].kind, dip::ValueChangeKind::Declaration);
    EXPECT_EQ(value.changes[1].kind, dip::ValueChangeKind::Modification);
    EXPECT_EQ(value.changes[1].location.line, 2);
    EXPECT_EQ(value.changes[2].location.line, 3);
    EXPECT_EQ(env["answer"].as<int>(), 3);

    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto snapshot = std::filesystem::temp_directory_path() / ("snt-inspection-history-" + suffix + ".diph5");
    env.save(snapshot);
    dip::reload_artifact(env, snapshot);
    value = dip::inspect_value(env, "answer");
    ASSERT_EQ(value.changes.size(), 3);
    EXPECT_EQ(value.changes[1].location.line, 2);
    EXPECT_EQ(value.changes[2].location.line, 3);
    std::filesystem::remove(snapshot);
}

TEST(Inspection, ReadOnlyTableViewSurvivesSnapshot) {
    dip::DIP parser;
    parser.add_string("measurements table = \"\"\"speed float m/s\ncount int\n---\n2 1\n3 2\n\"\"\"\n");
    parser.add_string("measurements.extra int[2] = [8, 9]\n");
    auto env = parser.parse();

    const auto table = dip::inspect_table(env, "measurements");
    EXPECT_EQ(table.rows, 2);
    ASSERT_EQ(table.columns.size(), 2);
    EXPECT_EQ(table.columns[0].name, "speed");
    EXPECT_EQ(table.columns[1].path, "measurements.count");
    EXPECT_TRUE(table.columns[0].units.has_value());
    EXPECT_TRUE(dip::inspect_capabilities(env, "measurements").hasTabularData);
    EXPECT_EQ(dip::inspect_value(env, "measurements.count").table_path, "measurements");
    EXPECT_TRUE(dip::inspect_value(env, "measurements.extra").table_path.empty());
    EXPECT_EQ(dip::inspect_tables(env).size(), 1);

    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto snapshot = std::filesystem::temp_directory_path() / ("snt-inspection-table-" + suffix + ".diph5");
    env.save(snapshot);
    dip::reload_artifact(env, snapshot);
    const auto restored = dip::inspect_table(env, "measurements");
    EXPECT_EQ(restored.rows, 2);
    ASSERT_EQ(restored.columns.size(), 2);
    EXPECT_EQ(restored.columns[0].name, "speed");
    EXPECT_EQ(restored.columns[1].name, "count");
    EXPECT_THROW(dip::inspect_table(env, "measurements.extra"), std::out_of_range);
    std::filesystem::remove(snapshot);
}
