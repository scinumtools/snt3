#include "pch_tests.h"
#include "snt/api/dip_parse.h"
#include "snt/api/exceptions.h"
#include "snt/dip/cursor.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace snt;

// TODO: test each argument

class DIPCommands : public ::testing::Test {
  protected:
    api::DIPParse cmd;

    void SetUp() override {
        // add a string input
        cmd.argument_add(
            "string",
            {"foo[bar]\n"
             "  jerk bool = true\n"
             "  snap int = 3\n"}
        );
    }
};

TEST_F(DIPCommands, AddFile) {

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
    cmd.argument_add("file", {example_filename.string()});
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
    cmd.argument_add("source", {"example", example_filename.string()});
    cmd.argument_add("string", {"sphinx float = {example?foo[baz].yippee.crackle}"});
    cmd.argument_print();

    std::string output = cmd.execute();
    EXPECT_NE(output.find("sphinx = 4e5"), std::string::npos);
}

TEST_F(DIPCommands, AddUnit) {
    // add a file input
    cmd.argument_add("unit", {"length", "km"});
    cmd.argument_add(
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

TEST_F(DIPCommands, ScalarBoolean) {
    cmd.argument_request("?foo[bar].jerk");
    cmd.argument_value("bool");
    EXPECT_EQ(cmd.execute(), "true\n");
}

TEST_F(DIPCommands, ScalarInteger) {
    cmd.argument_request("foo[bar].snap");
    cmd.argument_value("integer");
    EXPECT_EQ(cmd.execute(), "3\n");
}

TEST_F(DIPCommands, ScalarStringPreservesContent) {
    cmd.argument_add("string", {"text str = \"  a;b ${literal}  \"\n"});
    cmd.argument_request("text");
    cmd.argument_value("string");
    EXPECT_EQ(cmd.execute(), "  a;b ${literal}  \n");
}

TEST_F(DIPCommands, ScalarTypeMismatch) {
    cmd.argument_request("foo[bar].snap");
    cmd.argument_value("bool");
    EXPECT_THROW(cmd.execute(), std::exception);
}

TEST_F(DIPCommands, ScalarRejectsMultipleValues) {
    cmd.argument_request("foo[bar]");
    cmd.argument_value();
    EXPECT_THROW(cmd.execute(), std::exception);
}

TEST_F(DIPCommands, ScalarRejectsMissingValue) {
    cmd.argument_request("missing");
    cmd.argument_value();
    EXPECT_THROW(cmd.execute(), std::exception);
}

TEST_F(DIPCommands, ScalarRejectsArrayAndUnits) {
    for (const auto& source : {"array int[1] = [3]\n", "array float = 3 m\n", "array int\n"}) {
        api::DIPParse scalar;
        scalar.argument_add("string", {source});
        scalar.argument_request("array");
        scalar.argument_value();
        EXPECT_THROW(scalar.execute(), std::exception);
    }
}

class DIPPersistenceCommands : public DIPCommands {
  protected:
    std::filesystem::path file;

    void SetUp() override {
        DIPCommands::SetUp();
        file = std::filesystem::temp_directory_path() /
               (std::string("snt-api-") + ::testing::UnitTest::GetInstance()->current_test_info()->name() + ".diph5");
    }

    void TearDown() override { std::filesystem::remove(file); }

    void prepare_file() {
        dip::DIP parser;
        parser.add_string("simulation.steps int = 100\nsimulation.enabled bool = true\n");
        auto env = parser.parse();
        env.save(file);
    }
};

TEST_F(DIPPersistenceCommands, Save) {
    cmd.argument_save(file.string());
    EXPECT_EQ(cmd.execute(), "");
    ASSERT_TRUE(std::filesystem::exists(file));
    dip::Environment env;
    env.load(file);
    EXPECT_EQ(env["foo[bar].snap"].as<int64_t>(), 3);
    EXPECT_TRUE(env["foo[bar].jerk"].as<bool>());
}

TEST_F(DIPPersistenceCommands, Load) {
    prepare_file();
    api::DIPParse loaded;
    loaded.argument_load(file.string());
    loaded.argument_request("simulation.steps");
    loaded.argument_value("integer");
    EXPECT_EQ(loaded.execute(), "100\n");
}

TEST_F(DIPPersistenceCommands, SaveIgnoresOutputFilters) {
    cmd.argument_request("foo[bar].snap");
    cmd.argument_value("integer");
    cmd.argument_save(file.string());
    EXPECT_EQ(cmd.execute(), "3\n");
    dip::Environment env;
    env.load(file);
    EXPECT_TRUE(env["foo[bar].jerk"].as<bool>());
}

TEST_F(DIPPersistenceCommands, LoadAndSaveSameFile) {
    prepare_file();
    api::DIPParse loaded;
    loaded.argument_load(file.string());
    loaded.argument_save(file.string());
    loaded.argument_print();
    EXPECT_NE(loaded.execute().find("simulation.steps = 100"), std::string::npos);
    dip::Environment env;
    env.load(file);
    EXPECT_EQ(env["simulation.steps"].as<int64_t>(), 100);
}

TEST_F(DIPPersistenceCommands, FailedQueryDoesNotOverwrite) {
    prepare_file();
    cmd.argument_save(file.string());
    cmd.argument_request("foo[bar].snap");
    cmd.argument_value("bool");
    EXPECT_THROW(cmd.execute(), api::ArgumentException);
    dip::Environment env;
    env.load(file);
    EXPECT_EQ(env["simulation.steps"].as<int64_t>(), 100);
}

TEST_F(DIPPersistenceCommands, RejectsMixedInputs) {
    EXPECT_THROW(cmd.argument_load(file.string()), api::ArgumentException);
    for (const auto& kind : {"file", "string", "source", "unit"}) {
        api::DIPParse loaded;
        loaded.argument_load(file.string());
        EXPECT_THROW(loaded.argument_add(kind, {"unused"}), api::ArgumentException);
    }
}

TEST_F(DIPPersistenceCommands, RejectsEmptyPaths) {
    api::DIPParse loaded;
    EXPECT_THROW(loaded.argument_load(""), api::ArgumentException);
    EXPECT_THROW(cmd.argument_save(""), api::ArgumentException);
}

TEST_F(DIPPersistenceCommands, LoadFailure) {
    api::DIPParse loaded;
    loaded.argument_load((file / "missing.diph5").string());
    EXPECT_THROW(loaded.execute(), std::exception);
}

TEST_F(DIPPersistenceCommands, SaveFailure) {
    prepare_file();
    cmd.argument_save((file / "invalid.diph5").string());
    EXPECT_THROW(cmd.execute(), std::exception);
}
