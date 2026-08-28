#include "pch_tests.h"

#include <snt/api/exceptions.h>

using namespace snt;

TEST(Exceptions, BaseException) {

    try {
        throw api::Exception("message", "details", "suggestion", "source", 111);
        FAIL() << "Expected api::Exception";
    } catch (const api::Exception& e) {
        EXPECT_EQ(e.info().message, "message");
        EXPECT_EQ(e.info().details, "details");
        EXPECT_EQ(e.info().suggestion, "suggestion");
        EXPECT_EQ(e.info().origin->source, "source");
        EXPECT_EQ(e.info().origin->line, 111);
        EXPECT_STREQ(
            e.what(),
            "[SNT-API] message\n"
            "  details:    details\n"
            "  suggestion: suggestion\n"
            "  thrown:     source:111"
        );
    }

    try {
        throw api::Exception(
            core::ExceptionInfo{"message", "details", "suggestion", core::SourceLocation{"source", 111}}
        );
        FAIL() << "Expected api::Exception";
    } catch (const api::Exception& e) {
        EXPECT_EQ(e.info().message, "message");
        EXPECT_EQ(e.info().details, "details");
        EXPECT_EQ(e.info().suggestion, "suggestion");
        EXPECT_EQ(e.info().origin->source, "source");
        EXPECT_EQ(e.info().origin->line, 111);
        EXPECT_STREQ(
            e.what(),
            "[SNT-API] message\n"
            "  details:    details\n"
            "  suggestion: suggestion\n"
            "  thrown:     source:111"
        );
    }
}
