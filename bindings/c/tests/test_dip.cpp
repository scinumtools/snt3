#include <array>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <snt/c/dip.h>
#include <sstream>
#include <string>

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
