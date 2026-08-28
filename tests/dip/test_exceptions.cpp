#include "pch_tests.h"

#include <snt/dip/exceptions.h>
#include <snt/dip/settings.h>

using namespace snt;

TEST(Exceptions, BaseException) {

    try {
        throw dip::Exception(
            "message", "details", "suggestion", "source", 111, dip::Line{"code", dip::Source{"name", 222}}
        );
        FAIL() << "Expected dip::Exception";
    } catch (const dip::Exception& e) {
        EXPECT_EQ(e.info().message, "message");
        EXPECT_EQ(e.info().details, "details");
        EXPECT_EQ(e.info().suggestion, "suggestion");
        EXPECT_EQ(e.info().origin->source, "source");
        EXPECT_EQ(e.info().origin->line, 111);
        EXPECT_EQ(e.info().location->source, "name");
        EXPECT_EQ(e.info().location->line, 222);
        EXPECT_EQ(e.info().location->code, "code");
        EXPECT_STREQ(
            e.what(),
            "[SNT-DIP] message\n"
            "  details:    details\n"
            "  at:         name:222 | code\n"
            "  suggestion: suggestion\n"
            "  thrown:     source:111"
        );
    }

    try {
        throw dip::Exception(
            dip::ExceptionInfo{
                "message",
                "details",
                "suggestion",
                core::SourceLocation{"source", 111},
                core::SourceLocation{"name", 222, "code"}
            }
        );
        FAIL() << "Expected dip::Exception";
    } catch (const dip::Exception& e) {
        EXPECT_EQ(e.info().message, "message");
        EXPECT_EQ(e.info().details, "details");
        EXPECT_EQ(e.info().suggestion, "suggestion");
        EXPECT_EQ(e.info().origin->source, "source");
        EXPECT_EQ(e.info().origin->line, 111);
        EXPECT_EQ(e.info().location->source, "name");
        EXPECT_EQ(e.info().location->line, 222);
        EXPECT_EQ(e.info().location->code, "code");
        EXPECT_STREQ(
            e.what(),
            "[SNT-DIP] message\n"
            "  details:    details\n"
            "  at:         name:222 | code\n"
            "  suggestion: suggestion\n"
            "  thrown:     source:111"
        );
    }
}
