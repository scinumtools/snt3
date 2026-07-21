#include "snt/api/dip_parse.h"

#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace snt;

// TODO: test each argument

class DIPCommands : public ::testing::Test {
  protected:
    api::DIPParse cmd;

    void SetUp() override {
        cmd.argument_string(
            "foo[bar]\n"
            "  jerk bool = true\n"
            "  snap int = 3\n"
            "foo[baz]\n"
            "  yippee\n"
            "    crackle float = 4e5\n"
            "    pop str = \"shot\"\n"
            "  qux[]\n"
            "    jerk bool[3] = [true, false, true]\n"
            "    snap int[3] = [3,2,1]\n"
            "  qux[]\n"
            "    crackle float[2] = [4e5, 34e2]\n"
            "    pop str[2] = [\"shot\",\"puff\"]\n"
        );
    }
};

TEST_F(DIPCommands, Print) {

    cmd.argument_print();

    std::string output = cmd.execute();
    EXPECT_NE(output.find("foo[baz].yippee.crackle = 4e5"), std::string::npos);
}

TEST_F(DIPCommands, Request) {

    cmd.argument_request("foo[baz].yippee.crackle");
    cmd.argument_print();

    std::string output = cmd.execute();
    EXPECT_NE(output.find("crackle = 4e5"), std::string::npos);
}
