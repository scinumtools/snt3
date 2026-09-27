#include "pch_tests.h"
#include <fstream>

#include <snt/dip/dip.h>
#include <snt/dip/cursor.h>
#include <snt/dip/exceptions.h>

using namespace snt;

TEST(Modifications, DataTypeNode) {

    dip::DIP d;
    d.add_string("foo int = 2");
    d.add_string("foo int32 = 3");
    dip::Environment env = d.parse();

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->value_raw.at(0), "3");
    EXPECT_EQ(vnode->dtype, dip::NodeDtype::Integer);
    EXPECT_EQ(vnode->indent, 0);
    EXPECT_EQ(vnode->path.name, "foo");

    EXPECT_EQ(vnode->value->to_string(), "3");
    EXPECT_EQ(vnode->value->get_dtype(), core::DataType::Integer32);
}

TEST(Modifications, ModificationNode) {
    {
        dip::DIP d;
        d.add_string("foo int = 2");
        d.add_string("foo = 3");
        dip::Environment env = d.parse();

        dip::ValueNode::PointerType vnode = env.nodes.at(0);
        EXPECT_EQ(vnode->value_raw.at(0), "3");
        EXPECT_EQ(vnode->dtype, dip::NodeDtype::Integer);
        EXPECT_EQ(vnode->indent, 0);
        EXPECT_EQ(vnode->path.name, "foo");

        EXPECT_EQ(vnode->value->to_string(), "3");
        EXPECT_EQ(vnode->value->get_dtype(), core::DataType::Integer32);
    }
    { // in case modified node was not defined throw an exception
        dip::DIP d;
        d.add_string("foo = 3");
        try {
            d.parse();
            FAIL() << "Expected dip::SyntaxException";
        } catch (const dip::SyntaxException& e) {
            EXPECT_EQ(e.info().message, "Modifying undefined node");
            EXPECT_EQ(
                e.info().details, "The node type has not been defined and no previous node definition was found."
            );
            EXPECT_EQ(
                e.info().suggestion, "Specify a type such as `bool` or `float`, or add a declaration before this node."
            );
        } catch (...) {
            FAIL() << "Expected dip::SyntaxException";
        }
    }
}

TEST(Modifications, Declarations) {

    dip::DIP d;
    d.add_string("foo int");
    d.add_string("foo = 3");
    dip::Environment env = d.parse();

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->value_raw.at(0), "3");
    EXPECT_EQ(vnode->dtype, dip::NodeDtype::Integer);
    EXPECT_EQ(vnode->indent, 0);
    EXPECT_EQ(vnode->path.name, "foo");

    EXPECT_EQ(vnode->value->to_string(), "3");
    EXPECT_EQ(vnode->value->get_dtype(), core::DataType::Integer32);

    // if node is declared but has no value throw an exception
    d = dip::DIP();
    d.add_string("foo int");
    try {
        d.parse();
        FAIL() << "Expected dip::SyntaxException";
    } catch (const dip::SyntaxException& e) {
        EXPECT_EQ(e.info().message, "Undefined value");
        EXPECT_EQ(e.info().details, "The node has a value origin but no value has been defined.");
        EXPECT_EQ(e.info().suggestion, "Provide a valid value for the declared node.");
    } catch (...) {
        FAIL() << "Expected dip::SyntaxException";
    }
}

TEST(Modifications, Reference) {

    dip::DIP d;
    d.add_string(
        "foo int = 3\n"
        "bar int = 1\n"
        "bar = {?foo}"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 2);

    dip::ValueNode::PointerType vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->path.name, "bar");
    EXPECT_EQ(vnode->to_string(), "3");
}

TEST(Override, OrderDependenciesAndConstant) {
    // A region after the declarations still overrides a constant before dependents are evaluated.
    dip::DIP parser;
    parser.add_string(
        "radius float = 10 cm\n"
        "  !constant\n"
        "radius = 30 cm\n"
        "diameter float = ({?radius} * 2) cm\n"
        "$override\n"
        "  radius = 20 cm\n"
    );
    const auto env = parser.parse();
    EXPECT_EQ(env["radius"].as<double>(), 20);
    EXPECT_EQ(env["diameter"].as<double>(), 40);
    EXPECT_TRUE(env.get_node("radius")->constant);
    EXPECT_TRUE(env.get_node("radius")->override);
    EXPECT_EQ(env["radius"].get_provenance().override_line, 6);
}

TEST(Override, MultipleRegionsAndHostRegistration) {
    // All input forms share one override list, independent of their position among declarations.
    dip::DIP parser;
    parser.add_override_string("first = 5");
    const auto file = std::filesystem::temp_directory_path() / "snt-override-registration.dip";
    { std::ofstream output(file); output << "second = 6"; }
    parser.add_override_file(file);
    // Registration reads the body immediately; parsing no longer needs the file.
    std::filesystem::remove(file);
    parser.add_string(
        "$override\n"
        "  third = 7\n"
        "  fifth = 9\n"
        "first int = 1\n"
        "second int = 2\n"
        "$override\n"
        "  fourth = 8\n"
        "third int = 3\n"
        "fourth int = 4\n"
        "fifth int = 5\n"
    );
    const auto env = parser.parse();
    EXPECT_EQ(env["first"].as<int64_t>(), 5);
    EXPECT_EQ(env["second"].as<int64_t>(), 6);
    EXPECT_EQ(env["third"].as<int64_t>(), 7);
    EXPECT_EQ(env["fourth"].as<int64_t>(), 8);
    EXPECT_EQ(env["fifth"].as<int64_t>(), 9);
    EXPECT_TRUE(env.overrides.unresolved().empty());
}

TEST(Override, DuplicateAndMissingTargets) {
    // Targets are unique and must be instantiated, including when declarations are conditional.
    dip::DIP parser;
    parser.add_override_string("value = 2");
    EXPECT_THROW(parser.add_override_string("value = 2"), dip::SyntaxException);

    dip::DIP inline_parser;
    inline_parser.add_string("$override\n  value = 2\n$override\n  value = 2\nvalue int = 1\n");
    EXPECT_THROW(inline_parser.parse(), dip::SyntaxException);

    dip::DIP absent;
    absent.add_string("$override\n  missing = 2\nvalue int = 1\n");
    EXPECT_THROW(absent.parse(), dip::EnvironmentException);

    dip::DIP conditional;
    conditional.add_string("$override\n  missing = 2\n@if false\n  missing int = 1\n@end\n");
    EXPECT_THROW(conditional.parse(), dip::EnvironmentException);
}

TEST(Override, RejectsDefinitionsAndInvalidValues) {
    // Overrides replace values only and remain subject to the declaration's type, units, and options.
    dip::DIP definition;
    definition.add_string("$override\n  value int = 2\nvalue int = 1\n");
    EXPECT_THROW(definition.parse(), dip::SyntaxException);

    dip::DIP type;
    type.add_string("$override\n  value = \"wrong\"\nvalue int = 1\n");
    EXPECT_ANY_THROW(type.parse());

    dip::DIP dimensions;
    dimensions.add_string("$override\n  value = 2 s\nvalue float = 1 m\n");
    EXPECT_ANY_THROW(dimensions.parse());

    dip::DIP options;
    options.add_string("$override\n  value = 3\nvalue int = 1\n  !options [1, 2]\n");
    EXPECT_ANY_THROW(options.parse());
}

TEST(Override, SchemaMemberAndNestedPath) {
    // A fully qualified target can tune a member supplied by a schema.
    dip::DIP parser;
    parser.add_schema_string("settings", "value int = 1");
    parser.add_string("$override\n  physics.value = 10\nphysics : settings\n");
    const auto env = parser.parse();
    EXPECT_EQ(env["physics.value"].as<int64_t>(), 10);
    EXPECT_TRUE(env.get_node("physics.value")->override);
}

TEST(Override, SkipsOriginalValueFunction) {
    // The original value function must not run when an override supplies the effective value.
    dip::DIP parser;
    bool invoked = false;
    parser.add_function_value("original", [&](const dip::Environment&) -> dip::ValueNodeData {
        invoked = true;
        return {};
    });
    parser.add_string("value int = original()\n$override\n  value = 7\n");
    const auto env = parser.parse();
    EXPECT_FALSE(invoked);
    EXPECT_EQ(env["value"].as<int64_t>(), 7);
}

TEST(Override, DuplicateAcrossOriginsAndSchemaValidation) {
    // Input origin does not grant precedence or bypass constraints inherited from a schema.
    dip::DIP duplicate;
    duplicate.add_override_string("value = 2");
    duplicate.add_string("$override\n  value = 2\nvalue int = 1\n");
    EXPECT_THROW(duplicate.parse(), dip::SyntaxException);

    dip::DIP file_duplicate;
    file_duplicate.add_override_string("value = 2");
    const auto file = std::filesystem::temp_directory_path() / "snt-override-duplicate.dip";
    { std::ofstream output(file); output << "value = 3"; }
    EXPECT_THROW(file_duplicate.add_override_file(file), dip::SyntaxException);
    std::filesystem::remove(file);
    EXPECT_THROW(file_duplicate.add_override_file(file), dip::IOException);

    dip::DIP constrained;
    constrained.add_schema_string("limited", "value int = 1\n  !options [1, 2]");
    constrained.add_string("$override\n  record.value = 3\nrecord : limited\n");
    EXPECT_THROW(constrained.parse(), dip::SyntaxException);
}

TEST(Override, WholeArrayShapeValidation) {
    // Replace the whole array while enforcing its declared dimensions.
    dip::DIP valid;
    valid.add_string("$override\n  value = [3, 4]\nvalue int[2] = [1, 2]\n");
    EXPECT_EQ(valid.parse().get_node("value")->value->to_string(), "[3, 4]");

    dip::DIP invalid;
    invalid.add_string("$override\n  value = [3, 4, 5]\nvalue int[2] = [1, 2]\n");
    EXPECT_THROW(invalid.parse(), dip::SyntaxException);
}

TEST(Override, CollectionMemberPath) {
    // Match the concrete list index after the schema member is instantiated.
    dip::DIP parser;
    parser.add_schema_string("settings", "value int = 1");
    parser.add_string("$override\n  items[0].value = 9\nitems list : settings\nitems[]\n");
    const auto env = parser.parse();
    EXPECT_EQ(env["items[0].value"].as<int64_t>(), 9);
    EXPECT_TRUE(env.get_node("items[0].value")->override);
}

TEST(Override, NullReplacement) {
    // An explicit null replacement remains distinguishable from an untouched value.
    dip::DIP parser;
    parser.add_string("$override\n  value = none\nvalue int = 3\n");
    const auto env = parser.parse();
    EXPECT_EQ(env.get_node("value")->value, nullptr);
    EXPECT_TRUE(env.get_node("value")->override);
}

TEST(Override, InstantiatedConditionalTarget) {
    // A target in an active branch consumes its override normally.
    dip::DIP parser;
    parser.add_string("$override\n  value = 2\n@if true\n  value int = 1\n@end\n");
    const auto env = parser.parse();
    EXPECT_EQ(env.get_node("value")->value->to_string(), "2");
    EXPECT_TRUE(env.get_node("value")->override);
}

TEST(Override, ListTracksConsumption) {
    // Consumption resolves an entry without removing it or allowing another override for that path.
    dip::OverrideList overrides;
    auto node = std::make_shared<dip::BaseNode>(dip::NodeDtype::Modification);
    node->path = dip::Path("value");
    overrides.append(node);
    ASSERT_EQ(overrides.unresolved(), (std::vector<std::string>{"value"}));
    EXPECT_EQ(overrides.find("value"), node);
    EXPECT_EQ(overrides.find("other"), nullptr);
    overrides.consume("value");
    EXPECT_TRUE(overrides.unresolved().empty());
    EXPECT_THROW(overrides.append(node), dip::SyntaxException);
}

TEST(Override, ConstraintDiagnosticLocation) {
    // Preserve the exception category and declaration context, but point to the invalid override.
    dip::DIP parser;
    parser.add_string("value int = 1\n  !options [1, 2]");
    parser.add_override_string("value = 3");
    try {
        parser.parse();
        FAIL() << "Expected an invalid option";
    } catch (const dip::SyntaxException& exception) {
        const auto& info = exception.info();
        EXPECT_EQ(info.message, "Invalid option");
        ASSERT_TRUE(info.location);
        EXPECT_EQ(info.location->line, 1);
        EXPECT_EQ(info.location->code, "value = 3");
        EXPECT_NE(info.location->source.find("_OVERRIDE"), std::string::npos);
        EXPECT_NE(info.details.find("Declaration: "), std::string::npos);
        EXPECT_NE(info.details.find("value int = 1"), std::string::npos);
        EXPECT_FALSE(info.suggestion.empty());
        EXPECT_TRUE(info.origin);
    }

    // Without an override, the existing declaration diagnostic stays unchanged.
    dip::DIP ordinary;
    ordinary.add_string("value int = 3\n  !options [1, 2]");
    try {
        ordinary.parse();
        FAIL() << "Expected an invalid option";
    } catch (const dip::SyntaxException& exception) {
        ASSERT_TRUE(exception.info().location);
        EXPECT_EQ(exception.info().location->code, "value int = 3");
        EXPECT_EQ(exception.info().details.find("Declaration: "), std::string::npos);
    }
}
