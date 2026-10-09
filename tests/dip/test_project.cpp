#include "pch_tests.h"

#include <filesystem>
#include <fstream>
#include <snt/dip/cursor.h>
#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/inspect/inspector.h>
#include <string>
#include <utility>

using namespace snt;

namespace {

class ProjectDirectory {
  public:
    explicit ProjectDirectory(const std::string& name)
        : path_(std::filesystem::temp_directory_path() / ("scinumtools3-" + name)) {
        std::filesystem::remove_all(path_);
        std::filesystem::create_directories(path_);
    }

    ~ProjectDirectory() { std::filesystem::remove_all(path_); }

    const std::filesystem::path& path() const { return path_; }

    void write(const std::string& name, const std::string& contents) const {
        std::ofstream file(path_ / name);
        ASSERT_TRUE(file.is_open());
        file << contents;
    }

  private:
    std::filesystem::path path_;
};

} // namespace

TEST(Project, ParsesUnitsSourcesFilesAndStrings) {
    ProjectDirectory project("dip-project");
    project.write("constants.dip", "constant int = 7\n");
    project.write(
        "parameters.dip",
        "distance float = 2 length\n"
        "from_source int = {constants?constant}\n"
    );
    project.write(
        "DIPfile",
        "units[]\n"
        "  name = \"length\"\n"
        "  unit = \"23*au\"\n"
        "sources[]\n"
        "  name = \"constants\"\n"
        "  filepath = \"constants.dip\"\n"
        "code[]\n"
        "  file = \"parameters.dip\"\n"
        "code[]\n"
        "  string = \"\"\"\n"
        "derived int = ({?from_source} + 1)\n"
        "\"\"\"\n"
    );

    dip::DIP parser;
    parser.add_project(project.path() / "DIPfile");
    const dip::Environment env = parser.parse();

    EXPECT_EQ(env["distance"].as<double>(), 2.0);
    EXPECT_EQ(env["from_source"].as<int64_t>(), 7);
    EXPECT_EQ(env["derived"].as<int64_t>(), 8);

    const dip::Provenance provenance = env["derived"].get_provenance();
    ASSERT_TRUE(provenance.source.has_value());
    EXPECT_EQ(provenance.source->path, (project.path() / "DIPfile").string());
    EXPECT_NE(provenance.source->parent_name.find("_project"), std::string::npos);
}

TEST(Project, UnitDeclarationHasInspectableSource) {
    ProjectDirectory project("dip-unit-source-inspection");
    project.write("units.dip", "$unit local_length = 2*m\nlength float = 3 local_length\n");
    dip::DIP parser;
    parser.add_file(project.path() / "units.dip");
    const auto env = parser.parse();

    const auto locations = dip::Inspector{env}.source_locations({dip::SourceEntityKind::Unit, "local_length", {}});
    ASSERT_EQ(locations.size(), 1);
    EXPECT_EQ(locations.front().role, dip::SourceLocationRole::Definition);
    EXPECT_EQ(locations.front().source.path, (project.path() / "units.dip").string());
    EXPECT_EQ(locations.front().line, 1);
}

TEST(Project, LoadsOverrideFilesBeforeEvaluatingCode) {
    ProjectDirectory project("dip-project-overrides");
    std::filesystem::create_directories(project.path() / "tuning");
    project.write("tuning/values.dip", "settings.radius = 20 cm\n");
    project.write(
        "parameters.dip",
        "settings\n"
        "  radius float = 10 cm\n"
        "settings.radius = 30 cm\n"
        "diameter float = ({?settings.radius} * 2) cm\n"
    );
    project.write(
        "DIPfile",
        "code[]\n"
        "  file = \"parameters.dip\"\n"
        "overrides[]\n"
        "  file = \"tuning/values.dip\"\n"
    );

    dip::DIP parser;
    parser.add_project(project.path() / "DIPfile");
    const dip::Environment env = parser.parse();

    EXPECT_EQ(env["settings.radius"].as<double>(), 20);
    EXPECT_EQ(env["diameter"].as<double>(), 40);
    EXPECT_TRUE(env.get_node("settings.radius")->override);
    const auto provenance = env["settings.radius"].get_provenance();
    ASSERT_TRUE(provenance.override_source.has_value());
    ASSERT_TRUE(provenance.source.has_value());
    EXPECT_EQ(provenance.override_source->path, (project.path() / "tuning/values.dip").string());
    EXPECT_EQ(provenance.override_source->parent_name, provenance.source->parent_name);
    EXPECT_GT(provenance.override_source->parent_line, 0);
}

TEST(Project, RejectsDuplicateOverrideTargets) {
    ProjectDirectory project("dip-project-duplicate-overrides");
    project.write("first.dip", "answer = 10\n");
    project.write("second.dip", "answer = 20\n");
    project.write(
        "DIPfile",
        "overrides[]\n"
        "  file = \"first.dip\"\n"
        "overrides[]\n"
        "  file = \"second.dip\"\n"
        "code[]\n"
        "  string = \"answer int = 1\"\n"
    );

    dip::DIP parser;
    EXPECT_THROW(parser.add_project(project.path() / "DIPfile"), dip::SyntaxException);
}

TEST(Project, AllowsEmptyOverrideFile) {
    ProjectDirectory project("dip-project-empty-overrides");
    project.write("overrides.dip", "");
    project.write(
        "DIPfile",
        "overrides[]\n"
        "  file = \"overrides.dip\"\n"
        "code[]\n"
        "  string = \"answer int = 7\"\n"
    );

    dip::DIP parser;
    parser.add_project(project.path() / "DIPfile");
    const dip::Environment env = parser.parse();
    EXPECT_EQ(env["answer"].as<int64_t>(), 7);
    EXPECT_FALSE(env.get_node("answer")->override);
}

TEST(Project, RejectsMissingOverrideFile) {
    ProjectDirectory project("dip-project-missing-override");
    project.write("DIPfile", "overrides[]\n  file = \"missing.dip\"\n");

    dip::DIP parser;
    EXPECT_THROW(parser.add_project(project.path() / "DIPfile"), dip::IOException);
}

TEST(Project, RejectsUnknownOverrideField) {
    ProjectDirectory project("dip-project-invalid-override-field");
    project.write("DIPfile", "overrides[]\n  file = \"values.dip\"\n  string = \"answer = 2\"\n");

    dip::DIP parser;
    EXPECT_THROW(parser.add_project(project.path() / "DIPfile"), dip::ParserException);
}

TEST(Project, RequiresOverrideFileField) {
    ProjectDirectory project("dip-project-no-override-file-field");
    project.write("DIPfile", "overrides[]\n");

    dip::DIP parser;
    EXPECT_THROW(parser.add_project(project.path() / "DIPfile"), dip::Exception);
}

TEST(Project, RejectsAmbiguousCodeEntry) {
    ProjectDirectory project("dip-project-ambiguous");
    project.write(
        "DIPfile",
        "code[]\n"
        "  file = \"parameters.dip\"\n"
        "  string = \"answer int = 42\"\n"
    );

    dip::DIP parser;
    EXPECT_THROW(parser.add_project(project.path() / "DIPfile"), dip::SyntaxException);
}

TEST(Project, RegistersSchemaFilesAndStrings) {
    ProjectDirectory project("dip-project-schemas");
    project.write("settings.dipl", "?descr \"File schema\"\nvalue int = 42\n");
    project.write(
        "DIPfile",
        "schemas[]\n"
        "  name = \"from_file\"\n"
        "  file = \"settings.dipl\"\n"
        "schemas[]\n"
        "  name = \"from_string\"\n"
        "  string = \"\"\"\n"
        "?descr \"Inline schema\"\n"
        "value int = 43\n"
        "\"\"\"\n"
        "code[]\n"
        "  string = \"\"\"\n"
        "first : from_file\n"
        "second : from_string\n"
        "\"\"\"\n"
    );

    dip::DIP parser;
    parser.add_project(project.path() / "DIPfile");
    const dip::Environment env = parser.parse();
    const dip::Inspector view{env};

    EXPECT_EQ(env["first.value"].as<int64_t>(), 42);
    EXPECT_EQ(env["second.value"].as<int64_t>(), 43);
    EXPECT_EQ(env.schemas.entries().size(), 2);
    EXPECT_EQ(env.schemas.at("from_file").metadata.description, "File schema");
    EXPECT_EQ(env.schemas.at("from_string").metadata.description, "Inline schema");
    const auto& file_source = env.sources.at(env.schemas.at("from_file").nodes.at(0)->line.source.name);
    const auto& string_source = env.sources.at(env.schemas.at("from_string").nodes.at(0)->line.source.name);
    EXPECT_EQ(file_source.path, (project.path() / "settings.dipl").string());
    EXPECT_EQ(string_source.path, (project.path() / "DIPfile").string());
    EXPECT_EQ(file_source.parent.name, string_source.parent.name);
    EXPECT_FALSE(file_source.embedded);
    EXPECT_TRUE(string_source.embedded);
    EXPECT_EQ(env.schemas.at("from_file").registration_line, 3);
    EXPECT_EQ(env.schemas.at("from_string").registration_line, 6);
    EXPECT_EQ(env.schemas.at("from_file").registration_source_name, file_source.parent.name);
    EXPECT_EQ(env.schemas.at("from_string").registration_source_name, string_source.parent.name);
    const auto inline_locations = view.source_locations({dip::SourceEntityKind::Schema, "from_string", {}});
    ASSERT_EQ(inline_locations.size(), 2);
    EXPECT_EQ(inline_locations[0].role, dip::SourceLocationRole::Definition);
    EXPECT_EQ(inline_locations[1].role, dip::SourceLocationRole::Registration);
    EXPECT_EQ(inline_locations[0].line, 6);
    EXPECT_EQ(inline_locations[1].line, 6);
    EXPECT_EQ(inline_locations[0].logical_source_name, string_source.name);
    EXPECT_TRUE(inline_locations[0].embedded_registration);
    const auto value_locations = view.source_locations({dip::SourceEntityKind::Path, "second.value", {}});
    const auto inspected_value = view.value("second.value");
    const auto described_value = view.describe("second.value");
    ASSERT_FALSE(value_locations.empty());
    ASSERT_TRUE(described_value.declaration);
    EXPECT_EQ(inspected_value.declaration_location.source, described_value.declaration->source);
    EXPECT_EQ(inspected_value.declaration_location.line, described_value.declaration->line);
    EXPECT_EQ(described_value.declaration->line, value_locations.back().line);
    EXPECT_EQ(described_value.declaration->source, (project.path() / "DIPfile").string());
}

TEST(Project, RejectsAmbiguousSchemaEntry) {
    ProjectDirectory project("dip-project-ambiguous-schema");
    project.write(
        "DIPfile",
        "schemas[]\n"
        "  name = \"settings\"\n"
        "  file = \"settings.dipl\"\n"
        "  string = \"value int = 42\"\n"
    );

    dip::DIP parser;
    EXPECT_THROW(parser.add_project(project.path() / "DIPfile"), dip::SyntaxException);
}

TEST(Project, RejectsSchemaEntryWithoutBody) {
    ProjectDirectory project("dip-project-empty-schema-entry");
    project.write("DIPfile", "schemas[]\n  name = \"settings\"\n");

    dip::DIP parser;
    EXPECT_THROW(parser.add_project(project.path() / "DIPfile"), dip::SyntaxException);
}

TEST(Project, RejectsOrdinaryParameterNodes) {
    ProjectDirectory project("dip-project-invalid-node");
    project.write("DIPfile", "answer int = 42\n");

    dip::DIP parser;
    EXPECT_THROW(parser.add_project(project.path() / "DIPfile"), dip::SyntaxException);
}

TEST(Project, ParameterViewerExampleStaysBrowsable) {
    const auto project = std::filesystem::path(PROJECT_SOURCE_ROOT_DIR) /
                         "examples/dip/ParameterViewer/DIPfile";
    dip::DIP parser;
    parser.add_project(project);
    const dip::Environment env = parser.parse(true);
    const dip::Inspector view{env};

    EXPECT_DOUBLE_EQ(env["experiment.geometry.length"].as<double>(), 3.0);
    EXPECT_DOUBLE_EQ(env["experiment.average_speed"].as<double>(), 0.375);
    EXPECT_EQ(env["experiment.repeats.warmup"].as<int64_t>(), 1);
    EXPECT_EQ(env["experiment.first_sample_id"].as<int64_t>(), 10);
    EXPECT_EQ(env["experiment.calibration_revision"].as<int64_t>(), 3);
    EXPECT_EQ(env["experiment.summary"].as<std::string>(), "Run PV-2026-001 at North Lab");
    EXPECT_DOUBLE_EQ(env["experiment.target_temperature"].as<double>(), 295.0);
    EXPECT_EQ(env["state"].as<std::string>(), "fast");
    EXPECT_EQ(env.hierarchy.get_collection("experiment.materials").kind, dip::Path::Kind::Map);
    EXPECT_EQ(env.hierarchy.get_collection("experiment.probes").kind, dip::Path::Kind::List);
    EXPECT_TRUE(env.sources.at("reference").named_source);
    EXPECT_TRUE(env.sources.at("catalog").named_source);
    EXPECT_EQ(env.sources.at("catalog").hierarchy.get_collection("devices").kind, dip::Path::Kind::Map);
    EXPECT_EQ(env["experiment.instrument_family"].as<std::string>(), "Thermal");
    EXPECT_TRUE(env.get_node("experiment.geometry.length")->override);
    EXPECT_FALSE(env.get_applied_schemas("experiment.probes[1].accuracy").empty());
    EXPECT_EQ(env.get_node("experiment.readings.temperature")->value->get_shape().at(0), 4);
    EXPECT_TRUE(view.graph().recorded);

    const auto locations = [&](dip::SourceEntityKind kind, std::string name,
                               std::string source = {}, std::size_t index = 0) {
        return view.source_locations({kind, std::move(name), std::move(source), index});
    };
    const auto value = locations(dip::SourceEntityKind::Path, "experiment.geometry.length");
    ASSERT_EQ(value.size(), 2);
    EXPECT_EQ(value.front().role, dip::SourceLocationRole::Override);
    EXPECT_EQ(std::filesystem::path(value.front().source.path).generic_string(),
              (project.parent_path() / "overrides.dip").generic_string());
    EXPECT_EQ(value.back().role, dip::SourceLocationRole::Declaration);
    EXPECT_EQ(std::filesystem::path(value.back().source.path).generic_string(),
              (project.parent_path() / "parameters.dip").generic_string());

    const auto group = locations(dip::SourceEntityKind::Path, "experiment.geometry");
    ASSERT_EQ(group.size(), 1);
    EXPECT_EQ(group.front().role, dip::SourceLocationRole::Declaration);
    const auto source_group = locations(dip::SourceEntityKind::Path, "devices[thermometer]", "catalog");
    ASSERT_EQ(source_group.size(), 1);
    EXPECT_EQ(std::filesystem::path(source_group.front().source.path).generic_string(),
              (project.parent_path() / "catalog.dip").generic_string());

    const auto unit = locations(dip::SourceEntityKind::Unit, "sample_tick");
    ASSERT_EQ(unit.size(), 1);
    EXPECT_EQ(unit.front().line, 3);
    EXPECT_EQ(std::filesystem::path(unit.front().source.path).generic_string(), project.generic_string());
    const auto schema = locations(dip::SourceEntityKind::Schema, "probe");
    ASSERT_EQ(schema.size(), 2);
    EXPECT_EQ(schema.front().role, dip::SourceLocationRole::Definition);
    EXPECT_EQ(schema.back().role, dip::SourceLocationRole::Registration);
    EXPECT_EQ(schema.back().line, 14);
    const auto source = locations(dip::SourceEntityKind::NamedSource, "reference");
    ASSERT_EQ(source.size(), 1);
    EXPECT_EQ(std::filesystem::path(source.front().source.path).generic_string(),
              (project.parent_path() / "reference.dip").generic_string());
    const auto registration = locations(dip::SourceEntityKind::ProjectEntry, {}, {}, 1);
    ASSERT_EQ(registration.size(), 1);
    EXPECT_EQ(registration.front().line, 7);
}
