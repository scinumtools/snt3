#include "pch_tests.h"

#include <snt/dip/dip.h>

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
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "Value 17 of node 'space.resolution' doesn't match with any option: 8, 16, 32");
    } catch (...) {
        FAIL() << "Expected std::runtime_error";
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
            FAIL() << "Expected std::runtime_error";
        } catch (const std::runtime_error& e) {
            EXPECT_STREQ(e.what(), "Declared node has undefined value:   bar table");
        } catch (...) {
            FAIL() << "Expected std::runtime_error";
        }
    }
}
