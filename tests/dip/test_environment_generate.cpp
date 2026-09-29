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

    json.close();
    yaml.close();
    cpp.close();
    c.close();
    rust.close();
    julia.close();
    fortran.close();
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

    cpp.close();
    std::filesystem::remove(cpp_file);
}

TEST(Environment, GenerateValueNodeWithChildren) {
    dip::DIP parser;
    parser.add_string(
        "feature bool = true\n"
        "  setting int = 2\n"
    );
    const dip::Environment env = parser.parse();
    const auto json_file = environment_file("value-node-children.json");
    const auto yaml_file = environment_file("value-node-children.yaml");
    env.generate(dip::ExportFormat::JSON, json_file);
    env.generate(dip::ExportFormat::YAML, yaml_file);

    std::ifstream json(json_file);
    std::stringstream json_text;
    json_text << json.rdbuf();
    EXPECT_NE(json_text.str().find("\"feature\": {"), std::string::npos);
    EXPECT_NE(json_text.str().find("\"$value\": true"), std::string::npos);
    EXPECT_NE(json_text.str().find("\"setting\": 2"), std::string::npos);

    std::ifstream yaml(yaml_file);
    std::stringstream yaml_text;
    yaml_text << yaml.rdbuf();
    EXPECT_NE(yaml_text.str().find("feature:\n  $value: true\n  setting: 2"), std::string::npos);
    // Release file handles before deleting the files (required on Windows).
    json.close();
    yaml.close();
    std::filesystem::remove(json_file);
    std::filesystem::remove(yaml_file);
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

    cpp.close();
    c.close();
    rust.close();
    julia.close();
    fortran.close();
    std::filesystem::remove(cpp_file);
    std::filesystem::remove(c_file);
    std::filesystem::remove(rust_file);
    std::filesystem::remove(julia_file);
    std::filesystem::remove(fortran_file);
}

TEST(Environment, GeneratedFormatsPreserveTypedScientificInputs) {
    dip::DIP parser;
    parser.add_string(
        "run\n"
        "  enabled bool = true\n"
        "  signed8 int8 = -12\n"
        "  signed16 int16 = -1234\n"
        "  signed32 int32 = -123456\n"
        "  signed64 int64 = -9007199254740993\n"
        "  unsigned8 uint8 = 250\n"
        "  unsigned16 uint16 = 60000\n"
        "  unsigned32 uint32 = 4000000000\n"
        "  unsigned64 uint64 = 9007199254740993\n"
        "  fraction32 float32 = 1.25\n"
        "  fraction64 float64 = -2.5\n"
        "  label str = \"Flow \\\"study\\\"\"\n"
    );
    const auto env = parser.parse();
    const auto read = [&](dip::ExportFormat format, const std::string& suffix) {
        const auto file = environment_file("typed-export." + suffix);
        env.generate(format, file);
        std::ifstream stream(file);
        std::stringstream text;
        text << stream.rdbuf();
        stream.close();
        std::filesystem::remove(file);
        return text.str();
    };

    const auto cpp = read(dip::ExportFormat::CPP, "hpp");
    EXPECT_NE(cpp.find("std::int8_t signed8"), std::string::npos);
    EXPECT_NE(cpp.find("std::uint64_t unsigned64"), std::string::npos);
    EXPECT_NE(cpp.find("float fraction32"), std::string::npos);
    EXPECT_NE(cpp.find("9007199254740993"), std::string::npos);

    const auto c = read(dip::ExportFormat::C, "h");
    EXPECT_NE(c.find("int16_t signed16"), std::string::npos);
    EXPECT_NE(c.find("uint32_t unsigned32"), std::string::npos);
    EXPECT_NE(c.find("9007199254740993"), std::string::npos);

    const auto rust = read(dip::ExportFormat::RUST, "rs");
    EXPECT_NE(rust.find("pub signed64: i64"), std::string::npos);
    EXPECT_NE(rust.find("pub unsigned64: u64"), std::string::npos);
    EXPECT_NE(rust.find("9007199254740993"), std::string::npos);

    const auto julia = read(dip::ExportFormat::JULIA, "jl");
    EXPECT_NE(julia.find("Int8(-12)"), std::string::npos);
    EXPECT_NE(julia.find("UInt64(9007199254740993)"), std::string::npos);

    const auto fortran_file = environment_file("unsigned-fortran.f90");
    EXPECT_THROW(env.generate(dip::ExportFormat::FORTRAN, fortran_file), dip::EnvironmentException);
    std::filesystem::remove(fortran_file);

    dip::DIP signed_parser;
    signed_parser.add_string("signed8 int8 = -12\nsigned64 int64 = -9007199254740993\n"
                             "fraction32 float32 = 1.25\n");
    signed_parser.parse().generate(dip::ExportFormat::FORTRAN, fortran_file);
    std::ifstream fortran_stream(fortran_file);
    std::stringstream fortran_text;
    fortran_text << fortran_stream.rdbuf();
    fortran_stream.close();
    std::filesystem::remove(fortran_file);
    EXPECT_NE(fortran_text.str().find("integer(int8) :: signed8"), std::string::npos);
    EXPECT_NE(fortran_text.str().find("real(real32) :: fraction32"), std::string::npos);

    const auto json = read(dip::ExportFormat::JSON, "json");
    EXPECT_NE(json.find("\"unsigned64\": 9007199254740993"), std::string::npos);
    EXPECT_NE(json.find("\"label\": \"Flow \\\"study\\\"\""), std::string::npos);

    const auto yaml = read(dip::ExportFormat::YAML, "yaml");
    EXPECT_NE(yaml.find("unsigned64: 9007199254740993"), std::string::npos);
}
