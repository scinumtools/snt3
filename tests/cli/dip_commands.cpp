#include "snt/cli/dip_parse.h"

#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace snt;

TEST(DIPCommands, Parse) {

    cli::DIPParse cmd;
    cmd.argument_string("johan int = 32");
    cmd.argument_print();

    testing::internal::CaptureStdout();
    cmd.execute();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("johan = 32"), std::string::npos);
}
