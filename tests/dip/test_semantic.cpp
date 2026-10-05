#include <gtest/gtest.h>

#include <snt/dip/dip.h>
#include <snt/dip/inspect/semantic.h>
#include <snt/dip/preview.h>

#include <filesystem>

namespace {
const std::filesystem::path fixture =
    std::filesystem::path(PROJECT_SOURCE_ROOT_DIR) / "tests/dip/fixtures/generated_parameters.dip";
const std::filesystem::path rich_fixture =
    std::filesystem::path(PROJECT_SOURCE_ROOT_DIR) / "examples/dip/SemanticDescription/parameters.dip";
}

TEST(DIPSemantic, RichDescriptionWithRecordedReads) {
    snt::dip::DIP parser;
    parser.add_file(rich_fixture);
    const auto env = parser.parse(true);
    const auto value = snt::dip::describe(env, "simulation.speed");
    EXPECT_EQ(value.value_text, "3");
    EXPECT_EQ(value.units, "m*s-1");
    EXPECT_EQ(value.metadata.description, "Average speed calculated from distance and duration");
    EXPECT_EQ(value.metadata.authors, "Example Simulation Team");
    EXPECT_EQ(value.metadata.category, "Motion");
    EXPECT_EQ(value.metadata.requires,
              (std::vector<std::string>{"simulation.distance", "simulation.duration"}));
    EXPECT_EQ(value.tags, (std::vector<std::string>{"physics", "derived"}));
    EXPECT_EQ(value.enforced_condition, "{.} > 0 m/s");
    EXPECT_TRUE(value.dependencies_recorded);
    ASSERT_EQ(value.dependencies.size(), 2);
    EXPECT_EQ(value.dependencies[0].target, "?simulation.distance");
    EXPECT_EQ(value.dependencies[1].target, "?simulation.duration");
}

TEST(DIPSemantic, DescribeAndListBoundValues) {
    snt::dip::DIP parser;
    parser.add_file(fixture);
    const auto env = parser.parse();

    const auto value = snt::dip::describe(env, "experiment.steps");
    EXPECT_EQ(value.path, "experiment.steps");
    EXPECT_EQ(value.kind, "value");
    EXPECT_EQ(value.declared_type, "int32");
    EXPECT_EQ(value.stored_type, "int32");
    EXPECT_EQ(value.elements, 1);
    EXPECT_EQ(value.value_text, "4");
    EXPECT_TRUE(value.value_unavailable_reason.empty());
    EXPECT_TRUE(value.shape.empty());
    EXPECT_FALSE(value.units);
    EXPECT_TRUE(value.tags.empty());
    EXPECT_FALSE(value.overridden);
    EXPECT_FALSE(value.override_location);
    EXPECT_TRUE(value.source_text_available);
    EXPECT_TRUE(value.enforced_options.empty());
    EXPECT_TRUE(value.enforced_condition.empty());
    EXPECT_FALSE(value.dependencies_recorded);
    EXPECT_TRUE(value.dependencies.empty());
    ASSERT_TRUE(value.declaration);
    EXPECT_EQ(value.declaration->source, fixture.string());
    EXPECT_EQ(value.declaration->line, 3);
    const auto inspected = snt::dip::inspect_value(env, "experiment.steps");
    EXPECT_TRUE(value.shape.empty()); // Descriptions display scalars without the storage dimension.
    EXPECT_EQ(inspected.shape, (snt::val::Array::ShapeType{1}));
    EXPECT_EQ(value.tags, inspected.tags);
    EXPECT_EQ(value.metadata.description, inspected.metadata.description);
    EXPECT_EQ(value.declaration->source, inspected.declaration_location.source);

    const auto large = snt::dip::describe(env, "experiment.gains", 2);
    EXPECT_FALSE(large.value_text);
    EXPECT_EQ(large.value_unavailable_reason, "omitted_by_limit");
    EXPECT_EQ(large.elements, 3);
    EXPECT_EQ(large.shape, (snt::val::Array::ShapeType{3}));

    const auto listed = snt::dip::list_descriptions(env, "?experiment.", {}, 2);
    EXPECT_EQ(listed.total, 4);
    ASSERT_EQ(listed.items.size(), 2);
    EXPECT_EQ(listed.items[0].path, "experiment.title");
    EXPECT_FALSE(listed.items[0].value_text);
    EXPECT_EQ(listed.items[0].value_unavailable_reason, "omitted_by_limit");
    EXPECT_EQ(listed.items[1].path, "experiment.steps");
    EXPECT_EQ(listed.items[1].value_unavailable_reason, "omitted_by_limit");
    EXPECT_EQ(env.select_paths("?experiment.").size(), listed.total);
    EXPECT_EQ(snt::dip::describe(env, "experiment").kind, "group");
    EXPECT_EQ(snt::dip::describe(env, "sensors").kind, "collection");

    snt::dip::DIP long_text_parser;
    long_text_parser.add_string("label str = \"" + std::string(5000, 'a') + "\"");
    const auto long_text = snt::dip::describe(long_text_parser.parse(), "label");
    EXPECT_FALSE(long_text.value_text);
    EXPECT_EQ(long_text.value_unavailable_reason, "omitted_by_byte_limit");
}

TEST(DIPSemantic, SharedFactsStayAlignedForOverrides) {
    snt::dip::DIP parser;
    parser.add_string("speed float = 2 m/s\n  !tags [\"runtime\"]\n  ?descr \"Flow speed\"\n");
    parser.add_override_string("speed = 3 m/s\n");
    const auto env = parser.parse();
    const auto inspected = snt::dip::inspect_value(env, "speed");
    const auto described = snt::dip::describe(env, "speed");
    EXPECT_TRUE(described.shape.empty());
    EXPECT_EQ(inspected.shape, (snt::val::Array::ShapeType{1}));
    EXPECT_EQ(described.tags, inspected.tags);
    EXPECT_EQ(described.metadata.description, inspected.metadata.description);
    EXPECT_EQ(described.metadata.description, "Flow speed");
    EXPECT_EQ(described.tags, std::vector<std::string>{"runtime"});
    EXPECT_EQ(described.value_text, "3");
    EXPECT_EQ(described.units, inspected.units->to_string());
    EXPECT_TRUE(described.overridden);
    ASSERT_TRUE(described.override_location);
    ASSERT_TRUE(inspected.override_location);
    EXPECT_EQ(described.override_location->line, inspected.override_location->line);
    EXPECT_EQ(described.override_location->line, 1);
}

TEST(DIPSemantic, PreviewValidAndInvalidOverride) {
    const auto valid = snt::dip::preview(fixture, {
        {snt::dip::PreviewOverride::Kind::Text, "experiment.steps = 200"}
    });
    EXPECT_TRUE(valid.baseline_valid);
    EXPECT_TRUE(valid.candidate_valid);
    EXPECT_TRUE(valid.baseline_diagnostics.empty());
    EXPECT_TRUE(valid.candidate_diagnostics.empty());
    EXPECT_EQ(valid.comparison.scope, snt::dip::ComparisonScope::Effective);
    EXPECT_EQ(valid.comparison.added, 0);
    EXPECT_EQ(valid.comparison.removed, 0);
    EXPECT_EQ(valid.comparison.changed, 1);
    EXPECT_EQ(valid.accepted_override_targets, std::vector<std::string>{"experiment.steps"});
    ASSERT_EQ(valid.comparison.differences.size(), 1);
    const auto& difference = valid.comparison.differences[0];
    EXPECT_EQ(difference.path, "experiment.steps");
    EXPECT_EQ(difference.category, "value");
    EXPECT_EQ(difference.kind, snt::dip::DifferenceKind::Changed);
    EXPECT_EQ(difference.fields, std::vector<std::string>{"value"});
    EXPECT_EQ(difference.before, "4");
    EXPECT_EQ(difference.after, "200");
    EXPECT_EQ(difference.changed_elements, 1);

    const auto invalid = snt::dip::preview(fixture, {
        {snt::dip::PreviewOverride::Kind::Text, "missing = 2"}
    });
    EXPECT_TRUE(invalid.baseline_valid);
    EXPECT_FALSE(invalid.candidate_valid);
    EXPECT_TRUE(invalid.baseline_diagnostics.empty());
    ASSERT_EQ(invalid.candidate_diagnostics.size(), 1);
    const auto& diagnostic = invalid.candidate_diagnostics[0];
    EXPECT_EQ(diagnostic.code, "dip.environment");
    EXPECT_EQ(diagnostic.message, "Unresolved override");
    EXPECT_EQ(diagnostic.details, "The override target `missing` was not defined.");
    EXPECT_EQ(diagnostic.suggestion, "Define the node outside $override or remove the override.");
    EXPECT_FALSE(diagnostic.node_path);
    ASSERT_TRUE(diagnostic.location);
    EXPECT_EQ(diagnostic.location->line, 1);
    EXPECT_TRUE(invalid.accepted_override_targets.empty());
    EXPECT_EQ(invalid.comparison.added, 0);
    EXPECT_EQ(invalid.comparison.removed, 0);
    EXPECT_EQ(invalid.comparison.changed, 0);
    EXPECT_TRUE(invalid.comparison.differences.empty());

    const auto unchanged = snt::dip::preview(fixture, {});
    EXPECT_TRUE(unchanged.baseline_valid);
    EXPECT_TRUE(unchanged.candidate_valid);
    EXPECT_TRUE(unchanged.accepted_override_targets.empty());
    EXPECT_TRUE(unchanged.comparison.equal());

    snt::dip::DIP parser;
    parser.add_file(fixture);
    EXPECT_EQ(snt::dip::describe(parser.parse(), "experiment.steps").value_text, "4");
}
