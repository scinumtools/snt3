#include "pch_tests.h"

#include <snt/dip/cursor.h>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>

using namespace snt;

TEST(Environment, RequestValue) {

    dip::DIP d;
    d.add_string("foo bool = true");
    d.add_string("bar int = 3 J");
    dip::Environment env = d.parse();

    // request value as is
    val::BaseValue::PointerType node = env.request_value("?foo");
    EXPECT_EQ(node->to_string(), "true");

    // request value with a specific unit
    node = env.request_value("?bar", dip::RequestType::Reference, "erg");
    EXPECT_EQ(node->to_string(), "3e7");
}

TEST(Environment, RequestGroup) {

    dip::DIP d;
    d.add_string("foo.bar bool = true");
    d.add_string("  !tags [\"snap\",\"crackle\"]");
    d.add_string("foo.baz int = 3");
    d.add_string("  !tags [\"crackle\",\"pop\"]");
    dip::Environment env = d.parse();

    // select all children
    dip::ValueNode::ListType nodes = env.request_group("?foo");
    EXPECT_EQ(nodes.at(0)->to_string(), "true");
    EXPECT_EQ(nodes.at(1)->to_string(), "3");

    // filter selection using tags
    nodes = env.request_group("?foo", dip::RequestType::Reference, {"crackle"});
    EXPECT_EQ(nodes.size(), 2);
    EXPECT_EQ(nodes.at(0)->path.name, "bar");
    EXPECT_EQ(nodes.at(1)->path.name, "baz");

    nodes = env.request_group("?foo", dip::RequestType::Reference, {"snap"});
    EXPECT_EQ(nodes.size(), 1);
    EXPECT_EQ(nodes.at(0)->path.name, "bar");

    nodes = env.request_group("?foo", dip::RequestType::Reference, {"pop"});
    EXPECT_EQ(nodes.size(), 1);
    EXPECT_EQ(nodes.at(0)->path.name, "baz");
}

TEST(Environment, RequestMap) {

    dip::DIP d;
    d.add_string("jerk.snap[crackle]");
    d.add_string("  foo int = 3");
    d.add_string("jerk.snap[pop]");
    d.add_string("  bar int = 4");
    dip::Environment env = d.parse();

    // select all items
    std::unordered_map<std::string, dip::ValueNode::ListType> map = env.request_map("?jerk.snap");
    EXPECT_EQ(map.size(), 2);
    {
        auto it = map.find("crackle");
        EXPECT_NE(it, map.end());
        dip::ValueNode::ListType nodes = it->second;
        EXPECT_EQ(nodes.at(0)->path.name, "foo");
    }
    {
        auto it = map.find("pop");
        EXPECT_NE(it, map.end());
        dip::ValueNode::ListType nodes = it->second;
        EXPECT_EQ(nodes.at(0)->path.name, "bar");
    }
}

TEST(Environment, RequestList) {

    dip::DIP d;
    d.add_string("jerk.snap[]");
    d.add_string("  foo int = 3");
    d.add_string("jerk.snap[]");
    d.add_string("  bar int = 4");
    dip::Environment env = d.parse();

    // select all items
    std::vector<dip::ValueNode::ListType> list = env.request_list("?jerk.snap");
    EXPECT_EQ(list.size(), 2);
    {
        dip::ValueNode::ListType nodes = list.at(0);
        EXPECT_EQ(nodes.at(0)->path.name, "foo");
    }
    {
        dip::ValueNode::ListType nodes = list.at(1);
        EXPECT_EQ(nodes.at(0)->path.name, "bar");
    }
}

TEST(Environment, RequestNodeData) {

    dip::DIP d;
    d.add_string("foo.bar bool = true");
    d.add_string("foo.baz float = 3.45 J");
    dip::Environment env = d.parse();
    dip::ValueNodeData data = env.request_node_data("?foo.bar");
    EXPECT_TRUE(data.value);
    if (data.value) {
        EXPECT_EQ(data.value->to_string(), "true");
    }

    data = env.request_node_data("?foo.baz");
    EXPECT_TRUE(data.value && data.units);
    if (data.value) {
        EXPECT_EQ(data.value->to_string(), "3.45");
        EXPECT_EQ(data.units->to_string(), "J");
    }
}

TEST(Environment, RequestCursor) {

    dip::DIP d;
    d.add_string(
        "foo\n"
        "  bar bool = true\n"
        "  baz int = 3\n"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 2);

    {
        bool scalar = env["foo.bar"].as<bool>();
        EXPECT_EQ(scalar, true);
    }
    {
        int64_t scalar = env["foo.baz"].as<int64_t>();
        EXPECT_EQ(scalar, 3);
    }
}

// TODO: tests more requests with RequestType::Function
