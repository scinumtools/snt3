#include "pch_tests.h"

#include <snt/dip/dip.h>
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
