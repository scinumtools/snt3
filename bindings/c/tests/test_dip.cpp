#include <array>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <snt/c/dip.h>
#include <sstream>
#include <string>
#include <vector>

class Environment : public testing::Test {
  protected:
    snt_dip* dip = nullptr;
    snt_dip_error error{};

    void SetUp() override {
        ASSERT_EQ(snt_dip_parser_create(&dip, &error), 0);
        ASSERT_EQ(
            snt_dip_parser_add_string(
                dip,
                "title str = \"C ABI environment\"\n"
                "simulation\n"
                "  steps int = 100\n"
                "  timestep float = 0.5 fs\n"
                "  enabled bool = true",
                &error
            ),
            0
        );
        ASSERT_EQ(snt_dip_parser_parse(dip, &error), 0);
    }

    void TearDown() override { snt_dip_parser_free(dip); }
};

TEST(DIP, ParseAndGet) {
    snt_dip* dip = nullptr;
    snt_dip_error error{};

    ASSERT_EQ(snt_dip_parser_create(&dip, &error), 0);
    ASSERT_NE(dip, nullptr);
    ASSERT_EQ(snt_dip_parser_add_string(dip, "answer int = 42", &error), 0);
    ASSERT_EQ(snt_dip_parser_parse(dip, &error), 0);

    std::array<char, 64> output{};
    EXPECT_EQ(snt_dip_parser_get(dip, "answer", output.data(), output.size(), &error), 0);
    EXPECT_EQ(std::string(output.data()), "42");

    snt_dip_parser_free(dip);
}

TEST(DIP, TypedNodeTraversal) {
    snt_dip* dip = nullptr;
    snt_dip_error error{};
    ASSERT_EQ(snt_dip_parser_create(&dip, &error), 0);
    ASSERT_EQ(snt_dip_parser_add_string(
        dip,
        "simulation\n  enabled bool = true\n  timestep float = 0.5 fs\n  steps int = 42\n"
        "    !options [42,43]\n    !labels [\"42 steps\",\"43 steps\"]\n"
        "    ?rationale \"fixed step count\"\n    ?native [\"nsteps\"]\n"
        "  title str = \"run\"\n"
        "$schema integer\n  value int\n"
        "$schema edge\n  code int\n"
        "items list : integer\n"
        "items[]\n  value = 7\nitems[]\n  value = 9\n"
        "edges map : edge\nedges[first]\n  code = 11\n"
        "samples int[3] = [3,5,8]",
        &error), 0) << error.message;
    ASSERT_EQ(snt_dip_parser_parse(dip, &error), 0) << error.message;

    snt_dip_node* group = nullptr;
    ASSERT_EQ(snt_dip_environment_get_node(dip, "simulation", &group, &error), 0) << error.message;
    snt_dip_node_kind kind{};
    ASSERT_EQ(snt_dip_node_get_kind(group, &kind, &error), 0);
    EXPECT_EQ(kind, SNT_DIP_NODE_GROUP);
    size_t size = 0;
    ASSERT_EQ(snt_dip_node_get_size(group, &size, &error), 0);
    EXPECT_EQ(size, 4);

    snt_dip_node* steps = nullptr;
    ASSERT_EQ(snt_dip_node_get_child(group, "steps", &steps, &error), 0) << error.message;
    int64_t integer = 0;
    ASSERT_EQ(snt_dip_node_get_int64(steps, &integer, &error), 0) << error.message;
    EXPECT_EQ(integer, 42);
    snt_dip_value_type value_type{};
    ASSERT_EQ(snt_dip_node_get_type(steps, &value_type, &error), 0);
    EXPECT_EQ(value_type, SNT_DIP_VALUE_INT32);
    size_t rank = 99;
    ASSERT_EQ(snt_dip_node_get_shape(steps, nullptr, 0, &rank, &error), 0);
    EXPECT_EQ(rank, 1);
    size_t extent = 0;
    ASSERT_EQ(snt_dip_node_get_shape(steps, &extent, 1, &rank, &error), 0);
    EXPECT_EQ(extent, 1);
    size_t metadata_size = 0;
    ASSERT_EQ(snt_dip_node_get_metadata(steps, "rationale", nullptr, 0, &metadata_size, &error), 0);
    std::vector<char> rationale(metadata_size);
    ASSERT_EQ(snt_dip_node_get_metadata(steps, "rationale", rationale.data(), rationale.size(), &metadata_size, &error), 0);
    EXPECT_STREQ(rationale.data(), "fixed step count");
    int has_metadata = 0;
    ASSERT_EQ(snt_dip_node_has_metadata(steps, "rationale", &has_metadata, &error), 0);
    EXPECT_EQ(has_metadata, 1);
    ASSERT_EQ(snt_dip_node_has_metadata(steps, "title", &has_metadata, &error), 0);
    EXPECT_EQ(has_metadata, 0);
    ASSERT_EQ(snt_dip_node_get_metadata_list_size(steps, "native", &metadata_size, &error), 0);
    EXPECT_EQ(metadata_size, 1);
    std::array<char, 256> source_name{}, source_code{}, override_source_name{}, override_code{};
    size_t source_name_required = 0, source_code_required = 0;
    size_t override_source_name_required = 0, override_code_required = 0;
    size_t source_line = 0, override_line = 0;
    ASSERT_EQ(snt_dip_node_get_provenance(
        steps, source_name.data(), source_name.size(), &source_name_required, &source_line,
        source_code.data(), source_code.size(), &source_code_required,
        override_source_name.data(), override_source_name.size(), &override_source_name_required,
        &override_line, override_code.data(), override_code.size(), &override_code_required, &error), 0) << error.message;
    EXPECT_GT(source_line, 0);
    EXPECT_NE(std::string(source_code.data()).find("steps int = 42"), std::string::npos);
    EXPECT_EQ(override_line, 0);
    EXPECT_STREQ(override_source_name.data(), "");
    EXPECT_NE(snt_dip_node_get_bool(steps, nullptr, &error), 0);
    int overridden = -1;
    ASSERT_EQ(snt_dip_node_is_overridden(steps, &overridden, &error), 0);
    EXPECT_EQ(overridden, 0);
    size_t option_count = 0;
    ASSERT_EQ(snt_dip_node_get_option_count(steps, &option_count, &error), 0);
    EXPECT_EQ(option_count, 2);
    size_t option_text_size = 0;
    ASSERT_EQ(snt_dip_node_get_option_value(steps, 1, nullptr, 0, &option_text_size, &error), 0);
    std::vector<char> option_text(option_text_size);
    ASSERT_EQ(snt_dip_node_get_option_value(steps, 1, option_text.data(), option_text.size(), &option_text_size, &error), 0);
    EXPECT_STREQ(option_text.data(), "43");
    ASSERT_EQ(snt_dip_node_get_option_label(steps, 1, nullptr, 0, &option_text_size, &error), 0);
    option_text.resize(option_text_size);
    ASSERT_EQ(snt_dip_node_get_option_label(steps, 1, option_text.data(), option_text.size(), &option_text_size, &error), 0);
    EXPECT_STREQ(option_text.data(), "43 steps");

    snt_dip_node* timestep = nullptr;
    ASSERT_EQ(snt_dip_environment_get_node(dip, "simulation.timestep", &timestep, &error), 0);
    int has_units = 0;
    ASSERT_EQ(snt_dip_node_has_units(timestep, &has_units, &error), 0);
    EXPECT_EQ(has_units, 1);
    size_t unit_size = 0;
    ASSERT_EQ(snt_dip_node_get_units(timestep, nullptr, 0, &unit_size, &error), 0);
    std::vector<char> unit_text(unit_size);
    ASSERT_EQ(snt_dip_node_get_units(timestep, unit_text.data(), unit_text.size(), &unit_size, &error), 0);
    EXPECT_NE(std::string(unit_text.data()).find("fs"), std::string::npos);

    snt_dip_node* sequence = nullptr;
    ASSERT_EQ(snt_dip_environment_get_node(dip, "items", &sequence, &error), 0);
    ASSERT_EQ(snt_dip_node_get_size(sequence, &size, &error), 0);
    EXPECT_EQ(size, 2);
    snt_dip_node* element = nullptr;
    ASSERT_EQ(snt_dip_node_get_element(sequence, 1, &element, &error), 0) << error.message;
    snt_dip_node* list_value = nullptr;
    ASSERT_EQ(snt_dip_node_get_child(element, "value", &list_value, &error), 0);
    ASSERT_EQ(snt_dip_node_get_int64(list_value, &integer, &error), 0);
    EXPECT_EQ(integer, 9);
    snt_dip_node* mapping = nullptr;
    ASSERT_EQ(snt_dip_environment_get_node(dip, "edges", &mapping, &error), 0);
    ASSERT_EQ(snt_dip_node_get_size(mapping, &size, &error), 0);
    EXPECT_EQ(size, 1);
    size_t key_size = 0;
    ASSERT_EQ(snt_dip_node_get_key(mapping, 0, nullptr, 0, &key_size, &error), 0);
    std::vector<char> key(key_size);
    ASSERT_EQ(snt_dip_node_get_key(mapping, 0, key.data(), key.size(), &key_size, &error), 0);
    EXPECT_STREQ(key.data(), "first");
    snt_dip_node* item = nullptr;
    ASSERT_EQ(snt_dip_node_get_item(mapping, "first", &item, &error), 0) << error.message;
    snt_dip_node* code = nullptr;
    ASSERT_EQ(snt_dip_node_get_child(item, "code", &code, &error), 0) << error.message;
    size_t path_size = 0;
    ASSERT_EQ(snt_dip_node_get_path(code, nullptr, 0, &path_size, &error), 0);
    std::vector<char> path(path_size);
    ASSERT_EQ(snt_dip_node_get_path(code, path.data(), path.size(), &path_size, &error), 0);
    EXPECT_STREQ(path.data(), "edges[first].code");

    size_t required = 0;
    snt_dip_node* title = nullptr;
    ASSERT_EQ(snt_dip_node_get_child(group, "title", &title, &error), 0);
    EXPECT_EQ(snt_dip_node_get_string(title, nullptr, 0, &required, &error), 0);
    std::vector<char> text(required);
    ASSERT_EQ(snt_dip_node_get_string(title, text.data(), text.size(), &required, &error), 0);
    EXPECT_STREQ(text.data(), "run");

    snt_dip_node* samples = nullptr;
    ASSERT_EQ(snt_dip_environment_get_node(dip, "samples", &samples, &error), 0);
    ASSERT_EQ(snt_dip_node_get_value_size(samples, &size, &error), 0);
    EXPECT_EQ(size, 3);
    int64_t sample = 0;
    ASSERT_EQ(snt_dip_node_get_int64_at(samples, 2, &sample, &error), 0) << error.message;
    EXPECT_EQ(sample, 8);
    EXPECT_NE(snt_dip_node_get_int64_at(samples, 3, &sample, &error), 0);

    snt_dip_node_free(samples);
    snt_dip_node_free(timestep);
    snt_dip_node_free(code);
    snt_dip_node_free(item);
    snt_dip_node_free(mapping);
    snt_dip_node_free(element);
    snt_dip_node_free(sequence);
    snt_dip_node_free(list_value);
    snt_dip_node_free(steps);
    snt_dip_node_free(title);
    snt_dip_node_free(group);
    snt_dip_parser_free(dip);
}

TEST(DIP, ReportsErrors) {
    snt_dip_error error{};

    EXPECT_NE(snt_dip_parser_create(nullptr, &error), 0);
    EXPECT_NE(error.code, 0);
    ASSERT_NE(error.message, nullptr);
    EXPECT_NE(std::string(error.message).find("required"), std::string::npos);

    snt_dip_parser_free(nullptr);
}

TEST(DIP, Overrides) {
    // The C ABI reports registration errors and exposes override state after parsing or loading.
    snt_dip* parser = nullptr;
    snt_dip_error error{};
    ASSERT_EQ(snt_dip_parser_create(&parser, &error), 0);
    int overridden = -1;
    EXPECT_NE(snt_dip_parser_is_overridden(parser, "value", &overridden, &error), 0);
    EXPECT_NE(snt_dip_parser_add_override_string(parser, nullptr, &error), 0);
    EXPECT_NE(snt_dip_parser_add_override_file(parser, nullptr, &error), 0);
    ASSERT_EQ(snt_dip_parser_add_override_string(parser, "value = 20", &error), 0);
    const auto input = std::filesystem::temp_directory_path() / "snt-c-override-input.dip";
    { std::ofstream output(input); output << "other = 30"; }
    ASSERT_EQ(snt_dip_parser_add_override_file(parser, input.string().c_str(), &error), 0);
    std::filesystem::remove(input);
    EXPECT_NE(snt_dip_parser_add_override_file(parser, input.string().c_str(), &error), 0);
    EXPECT_NE(snt_dip_parser_add_override_string(parser, "value = 20", &error), 0);
    ASSERT_EQ(snt_dip_parser_add_string(parser, "value int = 1\n  !constant\nother int = 2\nplain int = 3", &error), 0);
    ASSERT_EQ(snt_dip_parser_parse(parser, &error), 0);
    std::array<char, 64> output{};
    ASSERT_EQ(snt_dip_parser_get(parser, "other", output.data(), output.size(), &error), 0);
    EXPECT_STREQ(output.data(), "30");
    ASSERT_EQ(snt_dip_parser_is_overridden(parser, "value", &overridden, &error), 0);
    EXPECT_EQ(overridden, 1);
    snt_dip_node* value_node = nullptr;
    ASSERT_EQ(snt_dip_environment_get_node(parser, "value", &value_node, &error), 0);
    ASSERT_EQ(snt_dip_node_is_overridden(value_node, &overridden, &error), 0);
    EXPECT_EQ(overridden, 1);
    snt_dip_node_free(value_node);
    ASSERT_EQ(snt_dip_parser_is_overridden(parser, "plain", &overridden, &error), 0);
    EXPECT_EQ(overridden, 0);
    EXPECT_NE(snt_dip_parser_is_overridden(parser, "missing", &overridden, &error), 0);
    EXPECT_NE(snt_dip_parser_is_overridden(parser, "value", nullptr, &error), 0);
    const auto file = std::filesystem::temp_directory_path() / "snt-c-overrides.diph5";
    ASSERT_EQ(snt_dip_environment_save(parser, file.string().c_str(), &error), 0);
    ASSERT_EQ(snt_dip_environment_load(parser, file.string().c_str(), &error), 0);
    ASSERT_EQ(snt_dip_parser_is_overridden(parser, "value", &overridden, &error), 0);
    EXPECT_EQ(overridden, 1);
    snt_dip_parser_free(parser);
    std::filesystem::remove(file);
}

TEST(DIP, ParseProject) {
    const auto directory = std::filesystem::temp_directory_path() / "scinumtools3-cabi-project";
    std::filesystem::remove_all(directory);
    ASSERT_TRUE(std::filesystem::create_directories(directory));
    {
        std::ofstream parameters(directory / "parameters.dip");
        parameters << "answer int = 42\n";
    }
    {
        std::ofstream project(directory / "DIPfile");
        project << "code[]\n"
                   "  file = \"parameters.dip\"\n";
    }

    snt_dip* dip = nullptr;
    snt_dip_error error{};
    ASSERT_EQ(snt_dip_parser_create(&dip, &error), 0);
    ASSERT_EQ(snt_dip_parser_add_project(dip, (directory / "DIPfile").string().c_str(), &error), 0);
    ASSERT_EQ(snt_dip_parser_parse(dip, &error), 0);

    std::array<char, 64> output{};
    EXPECT_EQ(snt_dip_parser_get(dip, "answer", output.data(), output.size(), &error), 0);
    EXPECT_EQ(std::string(output.data()), "42");

    snt_dip_parser_free(dip);
    std::filesystem::remove_all(directory);
}

TEST_F(Environment, Load) {
    const auto file = std::filesystem::temp_directory_path() / "scinumtools3-cabi-load.diph5";
    ASSERT_EQ(snt_dip_environment_save(dip, file.string().c_str(), &error), 0);

    snt_dip* loaded = nullptr;
    ASSERT_EQ(snt_dip_parser_create(&loaded, &error), 0);
    ASSERT_EQ(snt_dip_environment_load(loaded, file.string().c_str(), &error), 0);

    std::array<char, 64> output{};
    EXPECT_EQ(snt_dip_parser_get(loaded, "simulation.steps", output.data(), output.size(), &error), 0);
    EXPECT_EQ(std::string(output.data()), "100");

    snt_dip_parser_free(loaded);
    std::filesystem::remove(file);
}

TEST_F(Environment, Save) {
    const auto file = std::filesystem::temp_directory_path() / "scinumtools3-cabi-save.diph5";

    EXPECT_EQ(snt_dip_environment_save(dip, file.string().c_str(), &error), 0);
    EXPECT_TRUE(std::filesystem::is_regular_file(file));
    EXPECT_GT(std::filesystem::file_size(file), 0);

    std::filesystem::remove(file);
}

TEST(DIP, CompareFiles) {
    const auto before_file = std::filesystem::temp_directory_path() / "snt-c-compare-before.diph5";
    const auto after_file = std::filesystem::temp_directory_path() / "snt-c-compare-after.diph5";
    snt_dip_error error{};
    for (const auto& [file, code] : {std::pair{before_file, "value int = 1\n"},
                                     std::pair{after_file, "value int = 2\n"}}) {
        snt_dip* parser = nullptr;
        ASSERT_EQ(snt_dip_parser_create(&parser, &error), 0);
        ASSERT_EQ(snt_dip_parser_add_string(parser, code, &error), 0);
        ASSERT_EQ(snt_dip_parser_parse(parser, &error), 0);
        ASSERT_EQ(snt_dip_environment_save(parser, file.string().c_str(), &error), 0);
        snt_dip_parser_free(parser);
    }
    snt_dip_comparison* result = nullptr;
    ASSERT_EQ(snt_dip_compare_files(before_file.string().c_str(), after_file.string().c_str(),
                                    SNT_DIP_COMPARE_EFFECTIVE, 3, &result, &error), 0) << error.message;
    size_t added = 0, removed = 0, changed = 0;
    ASSERT_EQ(snt_dip_comparison_summary(result, &added, &removed, &changed, &error), 0);
    EXPECT_EQ(changed, 1);
    ASSERT_EQ(snt_dip_comparison_count(result), 1);
    snt_dip_difference difference{};
    ASSERT_EQ(snt_dip_comparison_get(result, 0, &difference, &error), 0);
    EXPECT_STREQ(difference.path, "value");
    EXPECT_EQ(difference.kind, SNT_DIP_DIFFERENCE_CHANGED);
    size_t required = 0;
    ASSERT_EQ(snt_dip_comparison_render_text(result, 50, nullptr, 0, &required, &error), 0);
    std::string text(required, '\0');
    ASSERT_EQ(snt_dip_comparison_render_text(result, 50, text.data(), text.size(), &required, &error), 0);
    EXPECT_NE(text.find("1 changed"), std::string::npos);
    snt_dip_comparison_free(result);
    std::filesystem::remove(before_file);
    std::filesystem::remove(after_file);
}

TEST_F(Environment, Generate) {
    const auto file = std::filesystem::temp_directory_path() / "scinumtools3-cabi-parameters.json";

    EXPECT_EQ(snt_dip_environment_generate(dip, SNT_DIP_EXPORT_JSON, file.string().c_str(), &error), 0);
    ASSERT_TRUE(std::filesystem::is_regular_file(file));
    std::ifstream generated(file);
    std::stringstream contents;
    contents << generated.rdbuf();
    EXPECT_NE(contents.str().find("\"simulation\""), std::string::npos);
    EXPECT_NE(contents.str().find("\"steps\": 100"), std::string::npos);

    generated.close();
    std::filesystem::remove(file);
}

#ifdef SNT_ENABLE_REPORT
TEST_F(Environment, GenerateReport) {
    const auto file = std::filesystem::temp_directory_path() / "scinumtools3-cabi-report.tex";
    ASSERT_EQ(snt_dip_environment_generate_report(
        dip, SNT_DIP_REPORT_TEX, file.string().c_str(), "C binding demo", nullptr, nullptr, &error), 0);
    std::ifstream generated(file);
    std::stringstream contents;
    contents << generated.rdbuf();
    EXPECT_NE(contents.str().find("simulation.steps"), std::string::npos);
    EXPECT_NE(contents.str().find("C binding demo"), std::string::npos);
    EXPECT_NE(contents.str().find("\\sntnode{simulation.steps}"), std::string::npos);
    generated.close();
    const snt_dip_report_options options{"C binding demo", nullptr, nullptr,
        "C & report", "Example_Team", "2026-09-28", "v1.0"};
    ASSERT_EQ(snt_dip_environment_generate_report_with_options(
        dip, SNT_DIP_REPORT_TEX, file.string().c_str(), &options, &error), 0);
    std::ifstream covered(file);
    std::stringstream covered_contents;
    covered_contents << covered.rdbuf();
    EXPECT_NE(covered_contents.str().find("C \\& report"), std::string::npos);
    EXPECT_NE(covered_contents.str().find("Example\\_Team"), std::string::npos);
    covered.close();
    const auto html_file = std::filesystem::temp_directory_path() / "scinumtools3-cabi-report.html";
    ASSERT_EQ(snt_dip_environment_generate_report(
        dip, SNT_DIP_REPORT_HTML, html_file.string().c_str(), nullptr, nullptr, nullptr, &error), 0);
    std::ifstream html(html_file);
    std::stringstream html_contents;
    html_contents << html.rdbuf();
    EXPECT_NE(html_contents.str().find("<html lang=\"en\">"), std::string::npos);
    EXPECT_NE(html_contents.str().find("simulation.steps"), std::string::npos);
    html.close();
    std::filesystem::remove(html_file);
    EXPECT_NE(snt_dip_environment_generate_report(
        dip, static_cast<snt_dip_report_format>(99), file.string().c_str(), nullptr, nullptr, nullptr, &error), 0);
    std::filesystem::remove(file);
}

TEST(DIPReport, RecordedCalculation) {
    snt_dip* parser = nullptr;
    snt_dip_error error{};
    ASSERT_EQ(snt_dip_parser_create(&parser, &error), 0);
    ASSERT_EQ(snt_dip_parser_add_string(parser, "base int = 3\ndouble int = ({?base} * 2)", &error), 0);
    ASSERT_EQ(snt_dip_parser_parse_with_graph(parser, 1, &error), 0);
    const auto file = std::filesystem::temp_directory_path() / "scinumtools3-cabi-graph-report.txt";
    ASSERT_EQ(snt_dip_environment_generate_report(parser, SNT_DIP_REPORT_TXT,
        file.string().c_str(), nullptr, nullptr, nullptr, &error), 0);
    std::ifstream generated(file);
    std::stringstream contents;
    contents << generated.rdbuf();
    EXPECT_NE(contents.str().find("Reads during evaluation | ?base"), std::string::npos);
    EXPECT_NE(contents.str().find("Used by | double"), std::string::npos);
    generated.close();
    std::filesystem::remove(file);
    snt_dip_parser_free(parser);
}
#else
TEST_F(Environment, ReportDisabled) {
    const auto file = std::filesystem::temp_directory_path() / "scinumtools3-cabi-disabled-report.tex";
    EXPECT_NE(snt_dip_environment_generate_report(
        dip, SNT_DIP_REPORT_TEX, file.string().c_str(), nullptr, nullptr, nullptr, &error), 0);
    ASSERT_NE(error.message, nullptr);
    EXPECT_NE(std::string(error.message).find("ENABLE_SNT_REPORT=ON"), std::string::npos);
}
#endif

TEST(DIP, RegisterSchema) {
    snt_dip* parser = nullptr;
    snt_dip_error error{};
    ASSERT_EQ(snt_dip_parser_create(&parser, &error), 0);
    EXPECT_NE(snt_dip_parser_add_schema_string(parser, nullptr, "value int = 1", &error), 0);
    ASSERT_EQ(snt_dip_parser_add_schema_string(parser, "settings", "value int = 42", &error), 0);
    ASSERT_EQ(snt_dip_parser_add_string(parser, "simulation : settings", &error), 0);
    ASSERT_EQ(snt_dip_parser_parse(parser, &error), 0);
    std::array<char, 64> output{};
    EXPECT_EQ(snt_dip_parser_get(parser, "simulation.value", output.data(), output.size(), &error), 0);
    EXPECT_EQ(std::string(output.data()), "42");
    snt_dip_parser_free(parser);
}

TEST(DIP, RegisterSchemaFile) {
    const auto file = std::filesystem::temp_directory_path() / "snt-c-schema-registration.dipl";
    { std::ofstream stream(file); stream << "value int = 7\n"; }
    snt_dip* parser = nullptr;
    snt_dip_error error{};
    ASSERT_EQ(snt_dip_parser_create(&parser, &error), 0);
    EXPECT_NE(snt_dip_parser_add_schema_file(parser, "settings", nullptr, &error), 0);
    ASSERT_EQ(snt_dip_parser_add_schema_file(parser, "settings", file.string().c_str(), &error), 0);
    ASSERT_EQ(snt_dip_parser_add_string(parser, "physics : settings", &error), 0);
    ASSERT_EQ(snt_dip_parser_parse(parser, &error), 0);
    std::array<char, 64> output{};
    EXPECT_EQ(snt_dip_parser_get(parser, "physics.value", output.data(), output.size(), &error), 0);
    EXPECT_EQ(std::string(output.data()), "7");
    snt_dip_parser_free(parser);
    std::filesystem::remove(file);
}
