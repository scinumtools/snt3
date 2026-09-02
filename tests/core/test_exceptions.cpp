#include <gtest/gtest.h>
#include <snt/core/exceptions.h>
#include <string>

using namespace snt;

TEST(Exceptions, Base) {

    try {
        throw core::Exception("message", "details\nsecond line", "suggestion\nsecond line", "source", 111);
        FAIL() << "Expected core::Exception";
    } catch (const core::Exception& e) {
        EXPECT_EQ(e.info().message, "message");
        EXPECT_EQ(e.info().details, "details\nsecond line");
        EXPECT_EQ(e.info().suggestion, "suggestion\nsecond line");
        EXPECT_EQ(e.info().origin->source, "source");
        EXPECT_EQ(e.info().origin->line, 111);
        EXPECT_STREQ(
            e.what(),
            "[SNT-CORE] message\n"
            "  details:    details\n"
            "              second line\n"
            "  suggestion: suggestion\n"
            "              second line\n"
            "  source:     source:111"
        );
    }
}
