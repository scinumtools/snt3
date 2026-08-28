#include "pch_tests.h"

#include <snt/exs/exceptions.h>

using namespace snt;

TEST(Exceptions, BaseException) {

    try {
        throw exs::Exception("message", "details", "suggestion", "source", 111);
        FAIL() << "Expected exs::Exception";
    } catch (const exs::Exception& e) {
        EXPECT_EQ(e.info().message, "message");
        EXPECT_EQ(e.info().details, "details");
        EXPECT_EQ(e.info().suggestion, "suggestion");
        EXPECT_EQ(e.info().origin->source, "source");
        EXPECT_EQ(e.info().origin->line, 111);
        EXPECT_STREQ(
            e.what(),
            "[SNT-EXS] message\n"
            "  details:    details\n"
            "  suggestion: suggestion\n"
            "  source:     source:111"
        );
    }

    try {
        throw exs::Exception(
            core::ExceptionInfo{"message", "details", "suggestion", core::SourceLocation{"source", 111}}
        );
        FAIL() << "Expected exs::Exception";
    } catch (const exs::Exception& e) {
        EXPECT_EQ(e.info().message, "message");
        EXPECT_EQ(e.info().details, "details");
        EXPECT_EQ(e.info().suggestion, "suggestion");
        EXPECT_EQ(e.info().origin->source, "source");
        EXPECT_EQ(e.info().origin->line, 111);
        EXPECT_STREQ(
            e.what(),
            "[SNT-EXS] message\n"
            "  details:    details\n"
            "  suggestion: suggestion\n"
            "  source:     source:111"
        );
    }
}
