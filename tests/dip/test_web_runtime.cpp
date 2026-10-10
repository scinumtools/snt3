#include <gtest/gtest.h>

#include "../../bindings/web/runtime.h"

TEST(WebRuntime, ParsesInspectsAndReevaluatesOverrides) {
    const std::string source =
        "simulation\n"
        "  distance float = 24 m\n"
        "  duration float = 8 s\n"
        "  speed float = ({?simulation.distance} / {?simulation.duration}) m/s\n";
    const snt::web::Model original(source);
    EXPECT_EQ(original.paths(), (std::vector<std::string>{
        "simulation.distance", "simulation.duration", "simulation.speed"}));
    EXPECT_EQ(original.describe("simulation.speed").value_text, "3");
    EXPECT_EQ(original.describe("simulation.speed").units, "m*s-1");

    const auto changed = original.with_override("simulation.distance = 32 m");
    EXPECT_EQ(changed.describe("simulation.speed").value_text, "4");
    EXPECT_EQ(original.describe("simulation.speed").value_text, "3");

    EXPECT_TRUE(snt::web::validate(source).valid);
    const auto invalid = snt::web::validate(source, "missing = 1");
    EXPECT_FALSE(invalid.valid);
    ASSERT_TRUE(invalid.diagnostic);
    EXPECT_FALSE(invalid.diagnostic->code.empty());
}

TEST(WebRuntime, ReusesSchemaInspection) {
    const snt::web::Model model("$schema settings\n  speed float = 2 m/s\nphysics : settings\n");
    const auto schemas = model.schemas();
    ASSERT_EQ(schemas.definitions.size(), 1);
    ASSERT_EQ(schemas.applications.size(), 1);
    EXPECT_EQ(schemas.applications[0].path, "physics");
    EXPECT_EQ(schemas.values[0].contributing_schema_id, "settings");
}

TEST(WebRuntime, ParsesInMemoryProjectAndRetainsCodeSource) {
    snt::dip::ProjectInput project;
    project.schemas.push_back({"settings", "distance float = 24 m\nduration float = 8 s"});
    project.code.push_back({"model.dip", "simulation : settings"});
    project.code.push_back({"derived.dip", "speed float = ({?simulation.distance} / {?simulation.duration}) m/s"});
    project.overrides.push_back({"preset.dip", "simulation.duration = 6 s"});
    const snt::web::Model model(project);
    EXPECT_EQ(model.describe("speed").value_text, "4");
    ASSERT_TRUE(model.describe("speed").declaration);
    EXPECT_EQ(model.describe("speed").declaration->source, "derived.dip");
    ASSERT_TRUE(model.describe("simulation.duration").override_location);
    EXPECT_EQ(model.describe("simulation.duration").override_location->source, "preset.dip");
    EXPECT_EQ(model.with_override("simulation.distance = 30 m").describe("speed").value_text, "5");
    const auto invalid = snt::web::validate(project, "unknown = 1");
    EXPECT_FALSE(invalid.valid);
    ASSERT_TRUE(invalid.diagnostic);
    EXPECT_FALSE(invalid.diagnostic->code.empty());
}
