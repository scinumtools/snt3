#include "pch_tests.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <snt/dip/cursor.h>
#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/inspect/inspector.h>
#include <snt/dip/project_bundle.h>
#include <snt/dip/project_init.h>
#include <stdexcept>
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
            std::ofstream file(path_ / name, std::ios::binary);
            ASSERT_TRUE(file.is_open());
            file << contents;
        }

      private:
        std::filesystem::path path_;
    };

} // namespace

TEST(Project, InitializeCreatesParseableStarterProject) {
    ProjectDirectory project("initialize-starter");
    dip::initialize_project(project.path());

    EXPECT_TRUE(std::filesystem::is_regular_file(project.path() / "DIPfile"));
    EXPECT_TRUE(std::filesystem::is_regular_file(project.path() / "main.dip"));
    dip::DIP parser;
    parser.add_project(project.path() / "DIPfile");
    EXPECT_EQ(parser.parse()["example"].as<int>(), 1);
}

TEST(Project, InitializePreservesExistingProjectFiles) {
    ProjectDirectory project("initialize-existing");
    project.write("main.dip", "existing int = 7\n");
    EXPECT_THROW(dip::initialize_project(project.path()), std::runtime_error);
    EXPECT_FALSE(std::filesystem::exists(project.path() / "DIPfile"));
    std::ifstream source(project.path() / "main.dip", std::ios::binary);
    EXPECT_EQ(std::string(std::istreambuf_iterator<char>(source), {}), "existing int = 7\n");
    source.close();

    std::filesystem::remove(project.path() / "main.dip");
    project.write("DIPfile", "existing manifest\n");
    EXPECT_THROW(dip::initialize_project(project.path()), std::runtime_error);
    EXPECT_FALSE(std::filesystem::exists(project.path() / "main.dip"));
}

TEST(Project, ConvertStandaloneSourceCreatesManifestWithoutChangingSource) {
    ProjectDirectory project("convert-standalone");
    project.write("parameters.dipl", "answer int = 7\n");
    const auto manifest = dip::create_project_from_source(project.path() / "parameters.dipl");
    EXPECT_EQ(manifest, project.path() / "DIPfile");
    EXPECT_EQ(dip::read_project_file(project.path() / "parameters.dipl"), "answer int = 7\n");
    EXPECT_EQ(dip::read_project_file(manifest), "code[]\n  file = \"parameters.dipl\"\n");
    dip::DIP parser;
    parser.add_project(manifest);
    EXPECT_EQ(parser.parse()["answer"].as<int>(), 7);
    EXPECT_THROW(dip::create_project_from_source(project.path() / "parameters.dipl"), std::runtime_error);
    EXPECT_EQ(dip::read_project_file(manifest), "code[]\n  file = \"parameters.dipl\"\n");
}

TEST(Project, ConvertStandaloneSourceRollsBackInvalidProject) {
    ProjectDirectory project("convert-invalid");
    project.write("broken.dip", "answer int = {missing?value}\n");
    EXPECT_THROW(dip::create_project_from_source(project.path() / "broken.dip"), std::exception);
    EXPECT_FALSE(std::filesystem::exists(project.path() / "DIPfile"));
    EXPECT_EQ(dip::read_project_file(project.path() / "broken.dip"), "answer int = {missing?value}\n");
}

TEST(Project, SourcesUseManifestDirectoryAndCodeSourcesUseTheirOwnFileDirectory) {
    ProjectDirectory project("relative-source-anchors");
    std::filesystem::create_directories(project.path() / "data");
    std::filesystem::create_directories(project.path() / "model");
    project.write(
        "DIPfile",
        "sources[]\n  name = \"registered\"\n  filepath = \"data/registered.dip\"\n"
        "code[]\n  file = \"model/main.dip\"\n"
        "code[]\n  string = \"\"\"\n"
        "$source inline = \"data/inline.dip\"\n"
        "from_inline int = {inline?value}\n"
        "\"\"\"\n"
    );
    project.write("data/registered.dip", "value int = 20\n");
    project.write("data/inline.dip", "value int = 24\n");
    project.write(
        "model/main.dip",
        "$source local = \"child.dip\"\n"
        "from_project int = {registered?value}\n"
        "from_local int = {local?value}\n"
    );
    project.write("model/child.dip", "value int = 22\n");

    dip::DIP parser;
    parser.add_project(project.path() / "DIPfile");
    const auto env = parser.parse();
    EXPECT_EQ(env["from_project"].as<int>(), 20);
    EXPECT_EQ(env["from_local"].as<int>(), 22);
    EXPECT_EQ(env["from_inline"].as<int>(), 24);
    EXPECT_EQ(
        std::filesystem::path(env.sources.at("registered").path).generic_string(),
        (project.path() / "data/registered.dip").generic_string()
    );
    EXPECT_EQ(
        std::filesystem::path(env.sources.at("local").path).generic_string(),
        (project.path() / "model/child.dip").generic_string()
    );
    EXPECT_EQ(
        std::filesystem::path(env.sources.at("inline").path).generic_string(),
        (project.path() / "data/inline.dip").generic_string()
    );
}

TEST(Project, PackagesTypedTableAndMatchesNativeEvaluation) {
    ProjectDirectory project("source-backed-bundle");
    project.write(
        "DIPfile",
        "units[]\n  name = \"tick\"\n  unit = \"s\"\n"
        "sources[]\n  name = \"measurements\"\n  filepath = \"measurements.dipt\"\n"
        "schemas[]\n  name = \"settings_schema\"\n  file = \"settings.dip\"\n"
        "overrides[]\n  file = \"preset.dip\"\n"
        "code[]\n  file = \"model.dip\"\n"
    );
    project.write("settings.dip", "factor float = 2\nmode int = 1\n");
    project.write("preset.dip", "settings.mode = 2\n");
    project.write(
        "model.dip",
        "settings : settings_schema\n"
        "samples table = {measurements}\n"
        "interval float = 3 tick\n"
        "scaled float = ({?settings.factor} * 3)\n"
    );
    project.write(
        "measurements.dipt",
        "?descr \"External measurement set\"\n"
        "time float s\n  ?descr \"Sample time\"\n"
        "mass float kg\n  ?descr \"Measured mass\"\n"
        "---\n0 1\n1 3\n"
    );

    dip::DIP native;
    native.add_project(project.path() / "DIPfile");
    const auto expected = native.parse();
    const auto bundle = dip::package_project(project.path() / "DIPfile");
    ASSERT_EQ(bundle.files.size(), 5);
    EXPECT_NE(dip::project_bundle_json(bundle).find("measurements.dipt"), std::string::npos);
    dip::DIP packaged;
    packaged.add_project(bundle);
    const auto actual = packaged.parse();
    EXPECT_EQ(actual["scaled"].as<double>(), expected["scaled"].as<double>());
    EXPECT_EQ(actual["settings.mode"].as<int64_t>(), expected["settings.mode"].as<int64_t>());
    EXPECT_EQ(actual["samples.time"].as<std::vector<double>>(), expected["samples.time"].as<std::vector<double>>());
    EXPECT_EQ(actual["samples.mass"].as<std::vector<double>>(), expected["samples.mass"].as<std::vector<double>>());
    const dip::Inspector inspected{actual};
    EXPECT_EQ(inspected.describe("samples").metadata.description, "External measurement set");
    const auto table = inspected.table("samples");
    ASSERT_EQ(table.columns.size(), 2);
    EXPECT_EQ(table.columns[0].metadata.description, "Sample time");
    EXPECT_EQ(table.columns[1].metadata.description, "Measured mass");
    EXPECT_EQ(dip::Inspector{actual}.describe("samples.mass").units, "kg");
    EXPECT_EQ(dip::Inspector{actual}.describe("scaled").value_text, "6");
    ASSERT_EQ(actual.project_entries().size(), expected.project_entries().size());
    for (size_t i = 0; i < actual.project_entries().size(); ++i) {
        EXPECT_EQ(actual.project_entries()[i].kind, expected.project_entries()[i].kind);
        EXPECT_EQ(actual.project_entries()[i].value, expected.project_entries()[i].value);
    }

    dip::DIP tuned;
    tuned.add_project(bundle);
    tuned.add_override_string("settings.factor = 4");
    EXPECT_EQ(tuned.parse()["scaled"].as<double>(), 12);
    project.write("alternate.dipt", "time float s\nmass float kg\n---\n0 5\n");
    EXPECT_EQ(dip::package_project(project.path() / "DIPfile", {"alternate.dipt"}).files.size(), 6);
}

TEST(Project, BundleRejectsMissingAndEscapingFiles) {
    const dip::ProjectBundle bundle{"DIPfile", {{"DIPfile", "code[]\n  file = \"missing.dip\"\n"}}};
    dip::DIP parser;
    EXPECT_THROW(parser.add_project(bundle), dip::IOException);
    dip::DIP traversal;
    const dip::ProjectBundle escaping_bundle{"DIPfile", {{"DIPfile", "code[]\n  file = \"../outside.dip\"\n"}}};
    EXPECT_THROW(traversal.add_project(escaping_bundle), dip::IOException);
    dip::DIP invalid_text;
    const dip::ProjectBundle invalid_bundle{"DIPfile", {{"DIPfile", std::string(1, static_cast<char>(0xff))}}};
    EXPECT_THROW(invalid_text.add_project(invalid_bundle), dip::IOException);
}

TEST(Project, ParsesCRLFManifestAndCode) {
    const dip::ProjectBundle bundle{
        "DIPfile",
        {{"DIPfile",
          "units[]\r\n  name = \"tick\"\r\n  unit = \"s\"\r\n"
          "code[]\r\n  file = \"model.dip\"\r\n"},
         {"model.dip", "value float = 2 tick\r\n"}}
    };
    dip::DIP parser;
    parser.add_project(bundle);
    EXPECT_DOUBLE_EQ(parser.parse()["value"].as<double>(), 2.0);
}

#ifndef _WIN32
TEST(Project, BundleKeepsLogicalSymlinkPath) {
    ProjectDirectory project("bundle-symlink");
    project.write(
        "DIPfile",
        "sources[]\n  name = \"data\"\n  filepath = \"alias.dipt\"\n"
        "code[]\n  string = \"samples table = {data}\"\n"
    );
    project.write("real.dipt", "mass float kg\n---\n3\n");
    std::filesystem::create_symlink("real.dipt", project.path() / "alias.dipt");
    const auto bundle = dip::package_project(project.path() / "DIPfile");
    EXPECT_NE(
        std::find_if(
            bundle.files.begin(), bundle.files.end(), [](const auto& file) { return file.path == "alias.dipt"; }
        ),
        bundle.files.end()
    );
    dip::DIP parser;
    parser.add_project(bundle);
    EXPECT_EQ(parser.parse()["samples.mass"].as<std::vector<double>>(), (std::vector<double>{3.0}));

    ProjectDirectory outside("bundle-outside");
    outside.write("outside.dipt", "mass float kg\n---\n9\n");
    std::filesystem::remove(project.path() / "alias.dipt");
    std::filesystem::create_symlink(outside.path() / "outside.dipt", project.path() / "alias.dipt");
    EXPECT_THROW(dip::package_project(project.path() / "DIPfile"), dip::IOException);
}
#endif

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
    const auto project = std::filesystem::path(PROJECT_SOURCE_ROOT_DIR) / "examples/dip/ParameterViewer/DIPfile";
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

    const auto locations =
        [&](dip::SourceEntityKind kind, std::string name, std::string source = {}, std::size_t index = 0) {
            return view.source_locations({kind, std::move(name), std::move(source), index});
        };
    const auto value = locations(dip::SourceEntityKind::Path, "experiment.geometry.length");
    ASSERT_EQ(value.size(), 2);
    EXPECT_EQ(value.front().role, dip::SourceLocationRole::Override);
    EXPECT_EQ(
        std::filesystem::path(value.front().source.path).generic_string(),
        (project.parent_path() / "overrides.dip").generic_string()
    );
    EXPECT_EQ(value.back().role, dip::SourceLocationRole::Declaration);
    EXPECT_EQ(
        std::filesystem::path(value.back().source.path).generic_string(),
        (project.parent_path() / "parameters.dip").generic_string()
    );

    const auto group = locations(dip::SourceEntityKind::Path, "experiment.geometry");
    ASSERT_EQ(group.size(), 1);
    EXPECT_EQ(group.front().role, dip::SourceLocationRole::Declaration);
    const auto source_group = locations(dip::SourceEntityKind::Path, "devices[thermometer]", "catalog");
    ASSERT_EQ(source_group.size(), 1);
    EXPECT_EQ(
        std::filesystem::path(source_group.front().source.path).generic_string(),
        (project.parent_path() / "sources/catalog.dip").generic_string()
    );

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
    EXPECT_EQ(
        std::filesystem::path(source.front().source.path).generic_string(),
        (project.parent_path() / "sources/reference.dip").generic_string()
    );
    const auto registration = locations(dip::SourceEntityKind::ProjectEntry, {}, {}, 1);
    ASSERT_EQ(registration.size(), 1);
    EXPECT_EQ(registration.front().line, 7);
}
