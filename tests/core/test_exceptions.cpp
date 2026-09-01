#include <gtest/gtest.h>
#include <snt/core/exceptions.h>
#include <string>

using namespace snt;

TEST(Exceptions, Base) {

    try {
        throw core::Exception("message", "details", "suggestion", "source", 111);
        FAIL() << "Expected core::Exception";
    } catch (const core::Exception& e) {
        EXPECT_EQ(e.info().message, "message");
        EXPECT_EQ(e.info().details, "details");
        EXPECT_EQ(e.info().suggestion, "suggestion");
        EXPECT_EQ(e.info().origin->source, "source");
        EXPECT_EQ(e.info().origin->line, 111);
        EXPECT_STREQ(
            e.what(),
            "[SNT-CORE] message\n"
            "  details:    details\n"
            "  suggestion: suggestion\n"
            "  source:     source:111"
        );
    }
}
