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
            "  source:     source:111"
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
            "  source:     source:111"
        );
    }
}

TEST(Exceptions, ActiveLineAppliesToMissingLocationsAndRestoresNestedScopes) {
    const dip::Line outer{"value str = {?missing}", {"main.dip", 3}};
    const dip::Line inner{"value str = {?other}", {"nested.dip", 7}};
    const auto check = [](const dip::Line* expected) {
        const dip::EnvironmentException error("Unknown source", "missing", "check", __FILE__, __LINE__);
        if (expected) {
            ASSERT_TRUE(error.info().location.has_value());
            EXPECT_EQ(error.info().location->source, expected->source.name);
            EXPECT_EQ(error.info().location->line, expected->source.line_number);
            EXPECT_EQ(error.info().location->code, expected->code);
        } else {
            EXPECT_FALSE(error.info().location.has_value());
        }
    };
    check(nullptr);
    {
        dip::ExceptionLocationScope outer_scope(outer);
        check(&outer);
        {
            dip::ExceptionLocationScope inner_scope(inner);
            check(&inner);
            const dip::SyntaxException explicit_line(
                "Explicit location", "", "", __FILE__, __LINE__, outer
            );
            ASSERT_TRUE(explicit_line.info().location.has_value());
            EXPECT_EQ(explicit_line.info().location->source, "main.dip");
        }
        check(&outer);
    }
    check(nullptr);
}
