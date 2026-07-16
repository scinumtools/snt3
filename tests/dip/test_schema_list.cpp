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
