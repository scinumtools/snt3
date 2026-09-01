#include <gtest/gtest.h>
#include <snt/core/settings.h>
#include <snt/core/to_string.h>
#include <string>

using namespace snt;

TEST(ToString, Precision) {

    EXPECT_EQ(core::to_string(1.23456789, 5), "1.2346");
}
