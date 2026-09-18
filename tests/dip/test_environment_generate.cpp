#include "pch_tests.h"
#include "test_environment_fixture.h"

#include <filesystem>
#include <fstream>
#include <sstream>

using namespace snt;

TEST(Environment, Generate) {
    dip::Environment env = parsed_environment();
    ASSERT_EQ(env.nodes.size(), 8);

    const auto json_file = environment_file("parameters.json");
    const auto yaml_file = environment_file("parameters.yaml");
    env.generate(dip::ExportFormat::JSON, json_file);
    env.generate(dip::ExportFormat::YAML, yaml_file);

    std::ifstream json(json_file);
    std::stringstream json_text;
    json_text << json.rdbuf();
    EXPECT_NE(json_text.str().find("\"simulation\""), std::string::npos);
    EXPECT_NE(json_text.str().find("\"inlet\""), std::string::npos);
    EXPECT_NE(json_text.str().find("\"samples\""), std::string::npos);

    std::ifstream yaml(yaml_file);
    std::stringstream yaml_text;
    yaml_text << yaml.rdbuf();
    EXPECT_NE(yaml_text.str().find("simulation:"), std::string::npos);
    EXPECT_NE(yaml_text.str().find("time: 0"), std::string::npos);

    std::filesystem::remove(json_file);
    std::filesystem::remove(yaml_file);
}
