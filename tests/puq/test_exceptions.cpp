#include "pch_tests.h"

#include <snt/puq/exceptions.h>

using namespace snt;

TEST(Exceptions, BaseException) {

    try {
        throw puq::Exception("message", "details", "suggestion", "source", 111);
        FAIL() << "Expected puq::Exception";
    } catch (const puq::Exception& e) {
        EXPECT_EQ(e.info().message, "message");
        EXPECT_EQ(e.info().details, "details");
        EXPECT_EQ(e.info().suggestion, "suggestion");
        EXPECT_EQ(e.info().origin->source, "source");
        EXPECT_EQ(e.info().origin->line, 111);
        EXPECT_STREQ(
            e.what(),
            "[SNT-PUQ] message\n"
            "  details:    details\n"
            "  suggestion: suggestion\n"
            "  thrown:     source:111"
        );
    }

    try {
        throw puq::Exception(
            core::ExceptionInfo{"message", "details", "suggestion", core::SourceLocation{"source", 111}}
        );
        FAIL() << "Expected puq::Exception";
    } catch (const puq::Exception& e) {
        EXPECT_EQ(e.info().message, "message");
        EXPECT_EQ(e.info().details, "details");
        EXPECT_EQ(e.info().suggestion, "suggestion");
        EXPECT_EQ(e.info().origin->source, "source");
        EXPECT_EQ(e.info().origin->line, 111);
        EXPECT_STREQ(
            e.what(),
            "[SNT-PUQ] message\n"
            "  details:    details\n"
            "  suggestion: suggestion\n"
            "  thrown:     source:111"
        );
    }
}
