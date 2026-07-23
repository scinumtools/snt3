#include "snt/api/dip_parse.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace snt;

// TODO: test each argument

class DIPCommands : public ::testing::Test {
  protected:
    api::DIPParse cmd;

    void SetUp() override {
        // add a string input
        cmd.argument_input(
            "string",
            {"foo[bar]\n"
             "  jerk bool = true\n"
             "  snap int = 3\n"}
        );
    }
};

TEST_F(DIPCommands, InputFile) {

    // create temporary file
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path example_filename = temp_dir / "example_file.dip";
    std::string example_code = ("foo[baz]\n"
                                "  yippee\n"
                                "    crackle float = 4e5\n"
                                "    pop str = \"shot\"\n"
                                "  qux[]\n"
                                "    jerk bool[3] = [true, false, true]\n"
                                "    snap int[3] = [3,2,1]\n"
                                "  qux[]\n"
                                "    crackle float[2] = [4e5, 34e2]\n"
                                "    pop str[2] = [\"shot\",\"puff\"]\n");
    {
        std::ofstream example_file(example_filename);
        ASSERT_TRUE(example_file.is_open()) << "Failed to create temp file.";
        example_file << example_code;
    }

    // add a file input
    cmd.argument_input("file", {example_filename.string()});
    cmd.argument_print();

    std::string output = cmd.execute();
    EXPECT_NE(output.find("foo[baz].yippee.crackle = 4e5"), std::string::npos);
}

TEST_F(DIPCommands, InputSource) {

    // create temporary file
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path example_filename = temp_dir / "example_file.dip";
    std::string example_code = ("foo[baz]\n"
                                "  yippee\n"
                                "    crackle float = 4e5\n"
                                "    pop str = \"shot\"\n"
                                "  qux[]\n"
                                "    jerk bool[3] = [true, false, true]\n"
                                "    snap int[3] = [3,2,1]\n"
                                "  qux[]\n"
                                "    crackle float[2] = [4e5, 34e2]\n"
                                "    pop str[2] = [\"shot\",\"puff\"]\n");
    {
        std::ofstream example_file(example_filename);
        ASSERT_TRUE(example_file.is_open()) << "Failed to create temp file.";
        example_file << example_code;
    }

    // add a file input
    cmd.argument_input("source", {"example", example_filename.string()});
    cmd.argument_input("string", {"sphinx float = {example?foo[baz].yippee.crackle}"});
    cmd.argument_print();

    std::string output = cmd.execute();
    EXPECT_NE(output.find("sphinx = 4e5"), std::string::npos);
}

TEST_F(DIPCommands, InputUnit) {
    // add a file input
    cmd.argument_input("unit", {"length", "km"});
    cmd.argument_input(
        "string",
        {"baz float cm\n"
         "baz = 1 length\n"}
    );
    cmd.argument_print();

    std::string output = cmd.execute();
    EXPECT_NE(output.find("baz = 1e5 cm"), std::string::npos);
}

TEST_F(DIPCommands, Print) {

    cmd.argument_print();

    std::string output = cmd.execute();
    EXPECT_NE(output.find("foo[bar].snap = 3"), std::string::npos);
}

TEST_F(DIPCommands, Request) {

    cmd.argument_request("foo[bar].snap");
    cmd.argument_print();

    std::string output = cmd.execute();
    EXPECT_NE(output.find("snap = 3"), std::string::npos);
}
