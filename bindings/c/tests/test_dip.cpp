#include <gtest/gtest.h>

#include <snt/c/dip.h>

#include <array>
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

    void TearDown() override {
        snt_dip_parser_free(dip);
    }
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

TEST_F(Environment, Load) {
    EXPECT_NE(snt_dip_environment_load(dip, "environment.h5", &error), 0);
    ASSERT_NE(error.message, nullptr);
    EXPECT_NE(std::string(error.message).find("Loading an environment from HDF5"), std::string::npos);
}

TEST_F(Environment, Save) {
    EXPECT_NE(snt_dip_environment_save(dip, "environment.h5", &error), 0);
    ASSERT_NE(error.message, nullptr);
    EXPECT_NE(std::string(error.message).find("Saving an environment to HDF5"), std::string::npos);
}

TEST_F(Environment, Generate) {
    EXPECT_NE(snt_dip_environment_generate(dip, SNT_DIP_OUTPUT_JSON, "parameters.json", &error), 0);
    ASSERT_NE(error.message, nullptr);
    EXPECT_NE(std::string(error.message).find("Generating a static parameter list"), std::string::npos);
}
