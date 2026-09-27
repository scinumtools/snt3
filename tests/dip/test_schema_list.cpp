#include "pch_tests.h"

#include <snt/dip/cursor.h>

#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>

using namespace snt;

TEST(SchemaList, ValueNodes) {

    dip::DIP d;
    d.add_string(
        "$schema human\n"
        "  alive bool\n"
        "  height float cm\n"
        "  weight int kg\n"
        "  name str\n"
        "jerk int = 1\n"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 1);

    dip::EnvSchema schema = env.schemas.at("human");
    EXPECT_EQ(schema.nodes.size(), 4);
    EXPECT_EQ(schema.id, schema.nodes.front()->line.source.name + "_SCHEMA0");
}

TEST(SchemaList, Properties) {
    dip::DIP d;
    d.add_string(
        "$schema simulation\n"
        "  resolution int\n"
        "    !options [16, 32, 64]\n"
        "dark_matter : simulation\n"
        "  resolution = 32\n"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 1);

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->path.name, "dark_matter.resolution");
    EXPECT_EQ(vnode->value->to_string(), "32");
}

TEST(SchemaList, Collections) {

    dip::DIP d;
    d.add_string(
        "$schema snack\n"
        "  fruit[apple]\n"
        "    color str\n"
        "    quantity int\n"
        "  beverage[]\n"
        "    hot bool\n"
        "    volume float l\n"
        "meal : snack\n"
        "  fruit[apple].color = \"red\"\n"
        "  fruit[apple].quantity = 1\n"
        "  beverage[0].hot = true\n"
        "  beverage[0].volume = 0.3\n"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 4);

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->path.name, "meal.fruit[apple].color");
    EXPECT_EQ(vnode->value->to_string(), "\"red\"");

    vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->path.name, "meal.fruit[apple].quantity");
    EXPECT_EQ(vnode->value->to_string(), "1");

    vnode = env.nodes.at(2);
    EXPECT_EQ(vnode->path.name, "meal.beverage[0].hot");
    EXPECT_EQ(vnode->value->to_string(), "true");

    vnode = env.nodes.at(3);
    EXPECT_EQ(vnode->path.name, "meal.beverage[0].volume");
    EXPECT_EQ(vnode->value->to_string(), "0.3");
    EXPECT_EQ(vnode->units->to_string(), "l");
}

TEST(SchemaList, Assignment) {

    // TODO: needs to be debugged

    dip::DIP d;
    d.add_string(
        "$schema car\n"
        "  new bool\n"
        "  speed float kph\n"
        "  weight int kg\n"
        "  vrn str\n"
        "jaguar : car\n"
        "  new = true\n"
        "  speed = 230.0\n"
        "  weight = 1450\n"
        "  vrn = \"AB23 XLM\"\n"
        "ford[focus] : car\n"
        "  new = false\n"
        "  speed = 213.0\n"
        "  weight = 1220\n"
        "  vrn = \"KT71 RPV\"\n"
        "suzuki[] : car\n"
        "  new = true\n"
        "  speed = 194.3\n"
        "  weight = 993\n"
        "  vrn = \"MH19 ZQD\"\n"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 12);

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->path.name, "jaguar.new");
    EXPECT_EQ(vnode->value->to_string(), "true");

    vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->path.name, "jaguar.speed");
    EXPECT_EQ(vnode->value->to_string(), "230");

    vnode = env.nodes.at(2);
    EXPECT_EQ(vnode->path.name, "jaguar.weight");
    EXPECT_EQ(vnode->value->to_string(), "1450");

    vnode = env.nodes.at(3);
    EXPECT_EQ(vnode->path.name, "jaguar.vrn");
    EXPECT_EQ(vnode->value->to_string(), "\"AB23 XLM\"");

    vnode = env.nodes.at(4);
    EXPECT_EQ(vnode->path.name, "ford[focus].new");
    EXPECT_EQ(vnode->value->to_string(), "false");

    vnode = env.nodes.at(5);
    EXPECT_EQ(vnode->path.name, "ford[focus].speed");
    EXPECT_EQ(vnode->value->to_string(), "213");

    vnode = env.nodes.at(6);
    EXPECT_EQ(vnode->path.name, "ford[focus].weight");
    EXPECT_EQ(vnode->value->to_string(), "1220");

    vnode = env.nodes.at(7);
    EXPECT_EQ(vnode->path.name, "ford[focus].vrn");
    EXPECT_EQ(vnode->value->to_string(), "\"KT71 RPV\"");

    vnode = env.nodes.at(8);
    EXPECT_EQ(vnode->path.name, "suzuki[0].new");
    EXPECT_EQ(vnode->value->to_string(), "true");

    vnode = env.nodes.at(9);
    EXPECT_EQ(vnode->path.name, "suzuki[0].speed");
    EXPECT_EQ(vnode->value->to_string(), "194.3");

    vnode = env.nodes.at(10);
    EXPECT_EQ(vnode->path.name, "suzuki[0].weight");
    EXPECT_EQ(vnode->value->to_string(), "993");

    vnode = env.nodes.at(11);
    EXPECT_EQ(vnode->path.name, "suzuki[0].vrn");
    EXPECT_EQ(vnode->value->to_string(), "\"MH19 ZQD\"");
}

TEST(SchemaList, Options) {

    dip::DIP d;
    d.add_string(
        "$schema box\n"
        "  resolution int\n"
        "    !options [8,16,32]\n"
        "space : box\n"
        "  resolution = 16\n"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 1);

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->options.size(), 3);
    EXPECT_EQ(vnode->options[0].value->to_string(), "8");
    EXPECT_EQ(vnode->options[1].value->to_string(), "16");
    EXPECT_EQ(vnode->options[2].value->to_string(), "32");

    // test if wrong options throw an exception

    d = dip::DIP();
    d.add_string(
        "$schema box\n"
        "  resolution int\n"
        "    !options [8,16,32]\n"
        "space : box\n"
        "  resolution = 17\n"
    );
    try {
        d.parse();
        FAIL() << "Expected dip::SyntaxException";
    } catch (const dip::SyntaxException& e) {
        EXPECT_EQ(e.info().message, "Invalid option");
        EXPECT_EQ(e.info().details, "The value `17` does not match any of the defined options: `8, 16, 32`.");
        EXPECT_EQ(e.info().suggestion, "Use one of the values defined by the node's `options` property.");
    } catch (...) {
        FAIL() << "Expected dip::SyntaxException";
    }
}

TEST(SchemaList, MultipleSchemas) {

    dip::DIP d;
    d.add_string(
        "$schema animal\n"
        "  sensation int = 19\n"
        "  memory int = 10\n"
        "$schema human\n"
        "  intellect int = 23\n"
        "  will int = 63\n"
        "john : animal, human\n"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 4);

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->path.name, "john.sensation");

    vnode = env.nodes.at(1);
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->path.name, "john.memory");

    vnode = env.nodes.at(2);
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->path.name, "john.intellect");

    vnode = env.nodes.at(3);
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->path.name, "john.will");
}

TEST(SchemaList, NestedSchemaModification) {
    dip::DIP d;
    d.add_string(
        "$schema child\n"
        "  x float = 1\n"
        "$schema parent\n"
        "  child : child\n"
        "root : parent\n"
        "  child\n"
        "    x = 2\n"
    );
    dip::Environment env = d.parse();

    ASSERT_EQ(env.nodes.size(), 1);
    dip::ValueNode::PointerType node = env.nodes.at(0);
    EXPECT_EQ(node->path.name, "root.child.x");
    EXPECT_EQ(node->value->to_string(), "2");
    EXPECT_EQ(node->value->get_dtype(), core::DataType::Float64);
}

TEST(SchemaList, FromCollection) {
    { // maps
        dip::DIP d;
        d.add_string(
            "$schema vehicle\n"
            "  speed float = 212 kph\n"
            "  weight float = 1320 kg\n"
            "$schema road\n"
            "  tires str = \"Continental\"\n"
            "vehicles map : vehicle\n"
            "vehicles[car] : road\n"
            "vehicles[ship]\n"
        );
        dip::Environment env = d.parse();
        EXPECT_EQ(env.nodes.size(), 5);

        dip::ValueNode::PointerType vnode = env.nodes.at(0);
        EXPECT_TRUE(vnode);
        EXPECT_EQ(vnode->path.name, "vehicles[car].speed");
        vnode = env.nodes.at(1);
        EXPECT_TRUE(vnode);
        EXPECT_EQ(vnode->path.name, "vehicles[car].weight");
        vnode = env.nodes.at(2);
        EXPECT_TRUE(vnode);
        EXPECT_EQ(vnode->path.name, "vehicles[car].tires");
        vnode = env.nodes.at(3);
        EXPECT_TRUE(vnode);
        EXPECT_EQ(vnode->path.name, "vehicles[ship].speed");
        vnode = env.nodes.at(4);
        EXPECT_TRUE(vnode);
        EXPECT_EQ(vnode->path.name, "vehicles[ship].weight");
    }
    { // list
        dip::DIP d;
        d.add_string(
            "$schema vehicle\n"
            "  speed float = 212 kph\n"
            "  weight float = 1320 kg\n"
            "$schema road\n"
            "  tires str = \"Continental\"\n"
            "vehicles list : vehicle\n"
            "vehicles[] : road\n"
            "vehicles[]\n"
        );
        dip::Environment env = d.parse();
        EXPECT_EQ(env.nodes.size(), 5);

        dip::ValueNode::PointerType vnode = env.nodes.at(0);
        EXPECT_TRUE(vnode);
        EXPECT_EQ(vnode->path.name, "vehicles[0].speed");
        vnode = env.nodes.at(1);
        EXPECT_TRUE(vnode);
        EXPECT_EQ(vnode->path.name, "vehicles[0].weight");
        vnode = env.nodes.at(2);
        EXPECT_TRUE(vnode);
        EXPECT_EQ(vnode->path.name, "vehicles[0].tires");
        vnode = env.nodes.at(3);
        EXPECT_TRUE(vnode);
        EXPECT_EQ(vnode->path.name, "vehicles[1].speed");
        vnode = env.nodes.at(4);
        EXPECT_TRUE(vnode);
        EXPECT_EQ(vnode->path.name, "vehicles[1].weight");
    }
}

TEST(SchemaList, DoubleUse) {
    // same schema was applied multiple times
    dip::DIP d;
    d.add_string(
        "$schema vehicle\n"
        "  speed float kph\n"
        "  weight float kg\n"
        "vehicles map : vehicle\n"
        "vehicles[porsche] : vehicle\n"
        "  speed = 212\n"
        "  weight = 1320\n"
    );
    try {
        d.parse();
        FAIL() << "Expected dip::SyntaxException";
    } catch (const dip::SyntaxException& e) {
        EXPECT_EQ(e.info().message, "Duplicated schema");
        EXPECT_EQ(e.info().details, "The schema `vehicle` is applied more than once to the same item.");
        EXPECT_EQ(
            e.info().suggestion,
            "The schema was probably declared both in the collection definition and on the item. Remove one of the "
            "duplicate schema declarations."
        );
    } catch (...) {
        FAIL() << "Expected dip::SyntaxException";
    }
}

TEST(SchemaList, TableDeclarations) {
    { // table is declared in the schema and defined later
        dip::DIP d;
        d.add_string(
            "$schema blup\n"
            "  baz int\n"
            "  bar table\n"
            "foo : blup\n"
            "  baz = 3\n"
            "  bar table = \"\"\"crackle int\n"
            "pop bool\n"
            "---\n"
            "1 true\n"
            "2 true\n"
            "3 false\n"
            "\"\"\""
        );
        dip::Environment env = d.parse();
        EXPECT_EQ(env.nodes.size(), 3);

        dip::ValueNode::PointerType vnode = env.nodes.at(0);
        EXPECT_TRUE(vnode);
        EXPECT_EQ(vnode->path.name, "foo.baz");
        vnode = env.nodes.at(1);
        EXPECT_TRUE(vnode);
        EXPECT_EQ(vnode->path.name, "foo.bar.crackle");
        vnode = env.nodes.at(2);
        EXPECT_TRUE(vnode);
        EXPECT_EQ(vnode->path.name, "foo.bar.pop");
    }
    { // table is declared in the schema but not defined later
        dip::DIP d;
        d.add_string(
            "$schema blup\n"
            "  baz int\n"
            "  bar table\n"
            "foo : blup\n"
            "  baz = 3\n"
        );
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
}

TEST(SchemaList, RejectValueWithAppliedSchema) {
    dip::DIP parser;
    parser.add_string(
        "$schema settings\n"
        "  child int = 1\n"
        "value int = 1 : settings\n"
    );
    EXPECT_THROW(parser.parse(), dip::SyntaxException);
}

TEST(SchemaList, HostRegistration) {
    dip::DIP parser;
    parser.add_schema_string("child", "value int = 2\n  !tags [\"export\"]\n  ?descr \"Child value\"\n");
    parser.add_schema_string("parent", "child : child\n");
    parser.add_string("settings : parent\n  child.value = 3\n");
    auto env = parser.parse();
    auto node = env.get_node("settings.child.value");
    EXPECT_EQ(node->value->to_string(), "3");
    EXPECT_EQ(node->metadata.description, "Child value");
    EXPECT_EQ(node->tags, std::vector<std::string>{"export"});
    EXPECT_EQ(env.schemas.at("child").nodes.front()->line.source.line_number, 1);
    EXPECT_NE(env.schemas.at("child").id.find("_STRING0_SCHEMA0"), std::string::npos);
    EXPECT_THROW(parser.add_schema_string("child", "value int = 1"), dip::EnvironmentException);
}

TEST(SchemaList, DeclarationMetadataStaysOnSchema) {
    dip::DIP parser;
    parser.add_string(
        "$schema settings\n"
        "  ?descr \"Reusable physics settings\"\n"
        "  ?since \"0.8.3\"\n"
        "  speed float = 2 m/s\n"
        "    ?descr \"Flow speed\"\n"
        "physics : settings\n"
    );
    const auto env = parser.parse();
    EXPECT_EQ(env.schemas.at("settings").metadata.description, "Reusable physics settings");
    EXPECT_EQ(env.schemas.at("settings").metadata.since, "0.8.3");
    EXPECT_EQ(env.get_node("physics.speed")->metadata.description, "Flow speed");
    EXPECT_TRUE(env.get_node("physics.speed")->metadata.since.empty());
}

TEST(SchemaList, HostRegistrationMetadata) {
    dip::DIP parser;
    parser.add_schema_string("settings", "?descr \"Reusable settings\"\nvalue int = 42\n");
    parser.add_string("physics : settings\n");
    const auto env = parser.parse();
    EXPECT_EQ(env.schemas.at("settings").metadata.description, "Reusable settings");
    EXPECT_EQ(env["physics.value"].as<int64_t>(), 42);
    EXPECT_TRUE(env.get_node("physics.value")->metadata.description.empty());
}

TEST(SchemaList, RejectNonMetadataOnSchema) {
    dip::DIP parser;
    parser.add_string("$schema settings\n  !tags [\"export\"]\n  value int = 42\n");
    EXPECT_THROW(parser.parse(), dip::SyntaxException);
    EXPECT_THROW(parser.add_schema_string("settings", "!tags [\"export\"]\nvalue int = 42"), dip::SyntaxException);
}

TEST(SchemaList, InvalidHostRegistration) {
    dip::DIP parser;
    EXPECT_THROW(parser.add_schema_string("bad name", "value int = 1"), dip::SyntaxException);
    EXPECT_THROW(parser.add_schema_string("empty", "# comment\n"), dip::SyntaxException);
    EXPECT_THROW(parser.add_schema_string("wrapper", "$schema inner\n  value int = 1"), dip::SyntaxException);
    EXPECT_THROW(parser.add_schema_string("indent", "  value int = 1"), dip::SyntaxException);
    EXPECT_THROW(parser.add_schema_string("property", "!tags [\"a\"]"), dip::SyntaxException);
    parser.add_schema_string("same", "value int = 1");
    parser.add_string("$schema same\n  value int = 2");
    EXPECT_THROW(parser.parse(), dip::EnvironmentException);
}

TEST(SchemaList, NestedUnassignedMemberInitializedByInstance) {
    // A schema declaration stays unassigned until the instance supplies its value.
    dip::DIP parser;
    parser.add_string(
        "$schema a\n"
        "  x str\n"
        "$schema b\n"
        "  sub : a\n"
        "y : b\n"
        "  sub\n"
        "    x = \"yes\"\n"
        "answer str = {?y.sub.x}\n"
    );
    const auto env = parser.parse();
    EXPECT_EQ(env["y.sub.x"].as<std::string>(), "yes");
    EXPECT_EQ(env["answer"].as<std::string>(), "yes");
    EXPECT_FALSE(env.get_node("y.sub.x")->override);
}

TEST(SchemaList, NestedUnassignedMemberOverriddenBeforeDependency) {
    // Nested override paths resolve against the concrete schema instance.
    dip::DIP parser;
    parser.add_string(
        "$schema a\n"
        "  x str\n"
        "    ?descr \"Nested setting\"\n"
        "$schema b\n"
        "  sub : a\n"
        "y : b\n"
        "  sub\n"
        "    x = \"yes\"\n"
        "answer str = {?y.sub.x}\n"
        "$override\n"
        "  y\n"
        "    sub\n"
        "      x = \"replacement\"\n"
    );
    const auto env = parser.parse();
    EXPECT_EQ(env["y.sub.x"].as<std::string>(), "replacement");
    EXPECT_EQ(env["answer"].as<std::string>(), "replacement");
    EXPECT_TRUE(env.get_node("y.sub.x")->override);
    EXPECT_EQ(env["y.sub.x"].get_provenance().override_code, "      x = \"replacement\"");
}

TEST(SchemaList, NestedValueChildSchemaApplication) {
    dip::DIP parser;
    parser.add_string(
        "$schema policy\n"
        "  enabled bool = true\n"
        "$schema inner\n"
        "  x float\n"
        "    export : policy\n"
        "$schema outer\n"
        "  sub : inner\n"
        "y : outer\n"
        "  sub\n"
        "    x = 1\n"
    );

    const auto env = parser.parse();
    EXPECT_EQ(env["y.sub.x"].as<double>(), 1);
    EXPECT_TRUE(env["y.sub.x.export.enabled"].as<bool>());
}

TEST(SchemaList, NestedValueChildSchemaIndependentInstances) {
    dip::DIP parser;
    parser.add_string(
        "$schema policy\n"
        "  enabled bool = true\n"
        "$schema inner\n"
        "  x float\n"
        "    export : policy\n"
        "$schema outer\n"
        "  sub : inner\n"
        "first : outer\n"
        "  sub\n"
        "    x = 1\n"
        "second : outer\n"
        "  sub\n"
        "    x = 2\n"
    );

    const auto env = parser.parse();
    EXPECT_EQ(env["first.sub.x"].as<double>(), 1);
    EXPECT_EQ(env["second.sub.x"].as<double>(), 2);
    EXPECT_TRUE(env["first.sub.x.export.enabled"].as<bool>());
    EXPECT_TRUE(env["second.sub.x.export.enabled"].as<bool>());
}

TEST(SchemaList, NestedValueChildSchemaWithHostOverride) {
    dip::DIP parser;
    parser.add_override_string("y.sub.x = 4");
    parser.add_string(
        "$schema policy\n"
        "  enabled bool = true\n"
        "$schema inner\n"
        "  x float\n"
        "    export : policy\n"
        "$schema outer\n"
        "  sub : inner\n"
        "y : outer\n"
        "  sub\n"
        "    x = 1\n"
        "twice float = ({?y.sub.x} * 2)\n"
    );

    const auto env = parser.parse();
    EXPECT_EQ(env["y.sub.x"].as<double>(), 4);
    EXPECT_EQ(env["twice"].as<double>(), 8);
    EXPECT_TRUE(env["y.sub.x.export.enabled"].as<bool>());
    EXPECT_TRUE(env.get_node("y.sub.x")->override);
    EXPECT_FALSE(env.get_node("y.sub.x.export.enabled")->override);
}

TEST(SchemaList, SeparateGroupSchemaDeclarationsApplyOnce) {
    dip::DIP parser;
    parser.add_string(
        "$schema first\n"
        "  one int = 1\n"
        "$schema second\n"
        "  two int = 2\n"
        "settings : first\n"
        "settings : second\n"
    );

    const auto env = parser.parse();
    EXPECT_EQ(env["settings.one"].as<int64_t>(), 1);
    EXPECT_EQ(env["settings.two"].as<int64_t>(), 2);
}

TEST(SchemaList, RepeatedGroupSchemaDeclarationStillFails) {
    dip::DIP parser;
    parser.add_string(
        "$schema first\n"
        "  one int = 1\n"
        "$schema second\n"
        "  two int = 2\n"
        "settings : first\n"
        "settings : second\n"
        "settings : first\n"
    );
    try {
        parser.parse();
        FAIL() << "Expected dip::SyntaxException";
    } catch (const dip::SyntaxException& error) {
        EXPECT_EQ(error.info().message, "Duplicated schema");
    } catch (...) {
        FAIL() << "Expected dip::SyntaxException";
    }
}
