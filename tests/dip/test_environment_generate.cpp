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
    const auto cpp_file = environment_file("parameters.hpp");
    const auto c_file = environment_file("parameters.h");
    const auto rust_file = environment_file("parameters.rs");
    const auto julia_file = environment_file("parameters.jl");
    const auto fortran_file = environment_file("parameters.f90");
    env.generate(dip::ExportFormat::JSON, json_file);
    env.generate(dip::ExportFormat::YAML, yaml_file);
    env.generate(dip::ExportFormat::CPP, cpp_file);
    env.generate(dip::ExportFormat::C, c_file);
    env.generate(dip::ExportFormat::RUST, rust_file);
    env.generate(dip::ExportFormat::JULIA, julia_file);
    env.generate(dip::ExportFormat::FORTRAN, fortran_file);

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

    std::ifstream cpp(cpp_file);
    std::stringstream cpp_text;
    cpp_text << cpp.rdbuf();
    EXPECT_NE(cpp_text.str().find("struct BoundaryEntry"), std::string::npos);
    EXPECT_NE(cpp_text.str().find("std::string_view key"), std::string::npos);
    EXPECT_NE(cpp_text.str().find("std::array<SamplesItem, 2> samples"), std::string::npos);
    EXPECT_NE(cpp_text.str().find("inline constexpr Parameters parameters"), std::string::npos);

    std::ifstream c(c_file);
    std::stringstream c_text;
    c_text << c.rdbuf();
    EXPECT_NE(c_text.str().find("typedef struct BoundaryEntry"), std::string::npos);
    EXPECT_NE(c_text.str().find("const char* key"), std::string::npos);

    std::ifstream rust(rust_file);
    std::stringstream rust_text;
    rust_text << rust.rdbuf();
    EXPECT_NE(rust_text.str().find("pub struct BoundaryEntry"), std::string::npos);
    EXPECT_NE(rust_text.str().find("pub static PARAMETERS"), std::string::npos);

    std::ifstream julia(julia_file);
    std::stringstream julia_text;
    julia_text << julia.rdbuf();
    EXPECT_NE(julia_text.str().find("const parameters = (;"), std::string::npos);
    EXPECT_NE(julia_text.str().find("\n  simulation = (;\n"), std::string::npos);
    EXPECT_NE(julia_text.str().find("\n    steps = Int32(100),\n"), std::string::npos);
    EXPECT_NE(julia_text.str().find("\"inlet\" =>"), std::string::npos);
    EXPECT_NE(julia_text.str().find("Int32(100)"), std::string::npos);

    std::ifstream fortran(fortran_file);
    std::stringstream fortran_text;
    fortran_text << fortran.rdbuf();
    EXPECT_NE(fortran_text.str().find("module snt_parameters"), std::string::npos);
    EXPECT_NE(fortran_text.str().find("type :: BoundaryEntry"), std::string::npos);
    EXPECT_NE(fortran_text.str().find("character(len=5) :: key"), std::string::npos);

    std::filesystem::remove(json_file);
    std::filesystem::remove(yaml_file);
    std::filesystem::remove(cpp_file);
    std::filesystem::remove(c_file);
    std::filesystem::remove(rust_file);
    std::filesystem::remove(julia_file);
    std::filesystem::remove(fortran_file);
}

TEST(Environment, GenerateMultiDimensionalArray) {
    dip::DIP parser;
    parser.add_string("tensor int[2,3] = [[1,2,3],[4,5,6]]");
    const dip::Environment env = parser.parse();
    const auto cpp_file = environment_file("tensor.hpp");

    env.generate(dip::ExportFormat::CPP, cpp_file);

    std::ifstream cpp(cpp_file);
    std::stringstream text;
    text << cpp.rdbuf();
    EXPECT_NE(
        text.str().find("std::array<std::array<std::int32_t, 3>, 2> tensor"),
        std::string::npos
    );
    EXPECT_NE(text.str().find("{{{{1, 2, 3}}, {{4, 5, 6}}}}"), std::string::npos);

    std::filesystem::remove(cpp_file);
}

TEST(Environment, GenerateThreeDimensionalArray) {
    dip::DIP parser;
    parser.add_string("volume int[2,2,3] = [[[1,2,3],[4,5,6]],[[7,8,9],[10,11,12]]]");
    const dip::Environment env = parser.parse();
    const auto cpp_file = environment_file("volume.hpp");
    const auto c_file = environment_file("volume.h");
    const auto rust_file = environment_file("volume.rs");
    const auto julia_file = environment_file("volume.jl");
    const auto fortran_file = environment_file("volume.f90");

    env.generate(dip::ExportFormat::CPP, cpp_file);
    env.generate(dip::ExportFormat::C, c_file);
    env.generate(dip::ExportFormat::RUST, rust_file);
    env.generate(dip::ExportFormat::JULIA, julia_file);
    env.generate(dip::ExportFormat::FORTRAN, fortran_file);

    std::ifstream cpp(cpp_file);
    std::stringstream cpp_text;
    cpp_text << cpp.rdbuf();
    EXPECT_NE(
        cpp_text.str().find("std::array<std::array<std::array<std::int32_t, 3>, 2>, 2> volume"),
        std::string::npos
    );
    EXPECT_NE(cpp_text.str().find("{{{{{{1, 2, 3}}, {{4, 5, 6}}}}, {{{{7, 8, 9}}, {{10, 11, 12}}}}}}"), std::string::npos);

    std::ifstream c(c_file);
    std::stringstream c_text;
    c_text << c.rdbuf();
    EXPECT_NE(c_text.str().find("int32_t volume[2][2][3]"), std::string::npos);

    std::ifstream rust(rust_file);
    std::stringstream rust_text;
    rust_text << rust.rdbuf();
    EXPECT_NE(rust_text.str().find("[[[i32; 3]; 2]; 2]"), std::string::npos);

    std::ifstream julia(julia_file);
    std::stringstream julia_text;
    julia_text << julia.rdbuf();
    EXPECT_NE(julia_text.str().find("volume = (\n"), std::string::npos);
    EXPECT_NE(julia_text.str().find("\n        Int32(1),\n"), std::string::npos);
    EXPECT_NE(julia_text.str().find("Int32(12)"), std::string::npos);

    std::ifstream fortran(fortran_file);
    std::stringstream fortran_text;
    fortran_text << fortran.rdbuf();
    EXPECT_NE(fortran_text.str().find("dimension(2, 2, 3) :: volume"), std::string::npos);
    EXPECT_NE(fortran_text.str().find("reshape([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12], [2, 2, 3], order=[3, 2, 1])"), std::string::npos);

    std::filesystem::remove(cpp_file);
    std::filesystem::remove(c_file);
    std::filesystem::remove(rust_file);
    std::filesystem::remove(julia_file);
    std::filesystem::remove(fortran_file);
}
