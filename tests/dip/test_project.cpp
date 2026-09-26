#include "pch_tests.h"

#include <filesystem>
#include <fstream>
#include <snt/dip/cursor.h>
#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>
#include <string>

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
