#include <gtest/gtest.h>
#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/tools/validator.h>

using snt::dip::tools::validate_text;

TEST(DipValidator, AcceptsValidDIPL) {
    const auto result = validate_text("answer int = 42\n", "valid.dip");
    EXPECT_TRUE(result.valid());
    EXPECT_TRUE(result.diagnostics.empty());
}

TEST(DipValidator, ReturnsStructuredParserError) {
    const auto result = validate_text("answer unknown_type = 42\n", "invalid.dip");
    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.size(), 1U);
    EXPECT_FALSE(result.diagnostics.front().code.empty());
    EXPECT_FALSE(result.diagnostics.front().message.empty());
}

TEST(DipValidator, CollectsIndependentSyntaxErrors) {
    const std::string source = "first unknown_type = 1\n"
                               "valid int = 2\n"
                               "second unknown_type = 3\n";
    const auto result = validate_text(source, "multiple.dip");
    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.size(), 2U);
    ASSERT_TRUE(result.diagnostics[0].location);
    ASSERT_TRUE(result.diagnostics[1].location);
    EXPECT_EQ(result.diagnostics[0].location->source, "multiple.dip");
    EXPECT_EQ(result.diagnostics[0].location->line, 1U);
    EXPECT_EQ(result.diagnostics[1].location->line, 3U);
}

TEST(DipValidator, NormalParsingRemainsFailFast) {
    snt::dip::DIP parser;
    parser.add_string("first unknown_type = 1\nsecond unknown_type = 2\n");
    EXPECT_THROW(parser.parse(), snt::dip::SyntaxException);
}

TEST(DipValidator, DoesNotRepeatInvalidContinuation) {
    const auto result = validate_text("first int\n= 1\nsecond unknown_type = 2\n", "continuation.dip");
    ASSERT_EQ(result.diagnostics.size(), 2U);
    ASSERT_TRUE(result.diagnostics[0].location);
    ASSERT_TRUE(result.diagnostics[1].location);
    EXPECT_EQ(result.diagnostics[0].location->line, 2U);
    EXPECT_EQ(result.diagnostics[1].location->line, 3U);
}

TEST(DipValidator, RecoversAfterValidMultilineString) {
    const auto result = validate_text(
        "message str = \"\"\"first line\nsecond line\"\"\"\n"
        "first unknown_type = 1\nsecond unknown_type = 2\n",
        "multiline.dip"
    );
    ASSERT_EQ(result.diagnostics.size(), 2U);
    ASSERT_TRUE(result.diagnostics[0].location);
    ASSERT_TRUE(result.diagnostics[1].location);
    EXPECT_EQ(result.diagnostics[0].location->line, 3U);
    EXPECT_EQ(result.diagnostics[1].location->line, 4U);
}

TEST(DipValidator, CapsCollectedSyntaxErrors) {
    snt::dip::DIP parser;
    parser.add_string("first unknown_type = 1\nsecond unknown_type = 2\nthird unknown_type = 3\n");
    EXPECT_EQ(parser.collect_syntax_diagnostics(2).size(), 2U);
}

TEST(DipValidator, StillEvaluatesSyntacticallyValidInput) {
    const auto result = validate_text("value int = 1\n  !options [2,3]\n", "semantic.dip");
    ASSERT_EQ(result.diagnostics.size(), 1U);
    EXPECT_EQ(result.diagnostics.front().message, "Invalid option");
}

TEST(DipValidator, HonorsDiagnosticLimitForInMemoryInput) {
    snt::dip::tools::ValidationOptions options;
    options.max_syntax_errors = 1;
    const auto result = validate_text("first unknown_type = 1\nsecond unknown_type = 2\n", "limited.dip", options);
    ASSERT_EQ(result.diagnostics.size(), 1U);
    ASSERT_TRUE(result.diagnostics.front().location);
    EXPECT_EQ(result.diagnostics.front().location->line, 1U);
}
