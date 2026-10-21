#include <gtest/gtest.h>
#include <snt/dip/dip.h>
#include <snt/dip/tools/linter.h>

using snt::dip::tools::lint_text;

TEST(DipLinter, CleanEvaluatedInput) {
    const auto result = lint_text("steps int = 8\n", "clean.dip");
    EXPECT_TRUE(result.input_valid);
    EXPECT_TRUE(result.clean());
}

TEST(DipLinter, ReportsPublicDescriptionAndDeprecationWithLocations) {
    const auto result = lint_text(
        "steps int = 8\n  ?visibility \"public\"\n"
        "old int = 3\n  ?deprecated \"Use steps instead\"\n  ?replacement \"steps\"\n",
        "advice.dip"
    );
    ASSERT_TRUE(result.input_valid);
    ASSERT_EQ(result.diagnostics.size(), 2U);
    EXPECT_EQ(result.diagnostics[0].code, "dip.lint.public-description");
    EXPECT_EQ(result.diagnostics[0].severity, snt::core::DiagnosticSeverity::Warning);
    EXPECT_TRUE(result.diagnostics[0].details.empty());
    ASSERT_TRUE(result.diagnostics[0].location);
    EXPECT_EQ(result.diagnostics[0].location->source, "advice.dip");
    EXPECT_EQ(result.diagnostics[0].location->line, 1U);
    ASSERT_TRUE(result.diagnostics[0].node_path);
    EXPECT_EQ(*result.diagnostics[0].node_path, "steps");
    EXPECT_EQ(result.diagnostics[1].code, "dip.lint.deprecated-value");
    EXPECT_EQ(result.diagnostics[1].suggestion, "Use `steps` instead.");
    ASSERT_TRUE(result.diagnostics[1].location);
    EXPECT_EQ(result.diagnostics[1].location->line, 3U);
}

TEST(DipLinter, InvalidInputReportsValidationErrorsOnly) {
    const auto result = lint_text("bad unknown_type = 1\nother unknown_type = 2\n", "invalid.dip");
    EXPECT_FALSE(result.input_valid);
    ASSERT_EQ(result.diagnostics.size(), 2U);
    EXPECT_EQ(result.diagnostics[0].severity, snt::core::DiagnosticSeverity::Error);
}

TEST(DipLinter, OptionsDisableAdvisoryRules) {
    snt::dip::tools::LintOptions options;
    options.require_public_descriptions = false;
    options.warn_deprecated_values = false;
    const auto result =
        lint_text("old int = 3\n  ?visibility \"public\"\n  ?deprecated \"Retired\"\n", "disabled.dip", options);
    EXPECT_TRUE(result.clean());
}

TEST(DipLinter, DescribedPublicValueIsClean) {
    const auto result =
        lint_text("steps int = 8\n  ?visibility \"public\"\n  ?descr \"Number of steps\"\n", "described.dip");
    EXPECT_TRUE(result.clean());
}

TEST(DipLinter, ChecksPublicSchemaDefinitionWithoutApplication) {
    snt::dip::DIP parser;
    parser.add_string("$schema reusable\n  ?visibility \"public\"\n  count int = 1\n", "schema.dip");
    const auto result = snt::dip::tools::lint_environment(parser.parse());
    ASSERT_EQ(result.diagnostics.size(), 1U);
    EXPECT_EQ(result.diagnostics[0].code, "dip.lint.public-description");
    ASSERT_TRUE(result.diagnostics[0].location);
    EXPECT_EQ(result.diagnostics[0].location->line, 1U);
}

TEST(DipLinter, ReportsRepeatedChoicesOncePerValue) {
    const auto result = lint_text("level int = 1\n  !options [1, 1, 2, 2]\n", "choices.dip");
    ASSERT_TRUE(result.input_valid);
    ASSERT_EQ(result.diagnostics.size(), 1U);
    EXPECT_EQ(result.diagnostics[0].code, "dip.lint.duplicate-options");
    EXPECT_EQ(result.diagnostics[0].details, "`1`, `2`");
    ASSERT_TRUE(result.diagnostics[0].location);
    EXPECT_EQ(result.diagnostics[0].location->line, 1U);
}

TEST(DipLinter, RequiresLabelsOnlyForPublicNumericChoices) {
    const auto result = lint_text(
        "level int = 1\n  ?visibility \"public\"\n  ?descr \"Level\"\n  !options [1, 2]\n"
        "mode str = \"fast\"\n  ?visibility \"public\"\n  ?descr \"Mode\"\n"
        "  !options [\"fast\", \"safe\"]\n"
        "private_level int = 1\n  !options [1, 2]\n",
        "labels.dip"
    );
    ASSERT_TRUE(result.input_valid);
    ASSERT_EQ(result.diagnostics.size(), 1U);
    EXPECT_EQ(result.diagnostics[0].code, "dip.lint.public-option-labels");
    ASSERT_TRUE(result.diagnostics[0].node_path);
    EXPECT_EQ(*result.diagnostics[0].node_path, "level");

    const auto labeled = lint_text(
        "level int = 1\n  ?visibility \"public\"\n  ?descr \"Level\"\n"
        "  !options [1, 2]\n  !labels [\"Low\", \"High\"]\n",
        "labeled.dip"
    );
    EXPECT_TRUE(labeled.clean());
}

TEST(DipLinter, ChecksOnlyClearLocalMetadataPathsWhenEnabled) {
    snt::dip::tools::LintOptions options;
    options.warn_broken_metadata_references = true;
    const auto result = lint_text(
        "root\n  child int = 1\n"
        "current int = 2\n  ?requires [\"root.child\", \"root.missing\"]\n"
        "  ?replacement \"read the new guide\"\n",
        "references.dip",
        options
    );
    ASSERT_TRUE(result.input_valid);
    ASSERT_EQ(result.diagnostics.size(), 1U);
    EXPECT_EQ(result.diagnostics[0].code, "dip.lint.broken-metadata-reference");
    EXPECT_EQ(result.diagnostics[0].details, "?requires references `root.missing`.");
}

TEST(DipLinter, UnusedSchemaRuleIsOptIn) {
    const std::string source = "$schema used\n  value int = 1\n"
                               "$schema spare\n  value int = 2\n"
                               "item : used\n";
    EXPECT_TRUE(lint_text(source, "schemas.dip").clean());
    snt::dip::tools::LintOptions options;
    options.warn_unused_schemas = true;
    const auto result = lint_text(source, "schemas.dip", options);
    ASSERT_TRUE(result.input_valid);
    ASSERT_EQ(result.diagnostics.size(), 1U);
    EXPECT_EQ(result.diagnostics[0].code, "dip.lint.unused-schema");
    ASSERT_TRUE(result.diagnostics[0].node_path);
    EXPECT_EQ(*result.diagnostics[0].node_path, "spare");
}
