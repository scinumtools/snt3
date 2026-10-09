#include "pch_tests.h"

#include <snt/dip/inspect/inspector.h>
#include <snt/dip/cursor.h>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>
#include <snt/dip/nodes/node_integer.h>
#include <snt/val/values.h>

using namespace snt;

TEST(Environment, RequestValue) {
    dip::DIP d;
    d.add_string("foo bool = true");
    d.add_string("bar int = 3 J");
    dip::Environment env = d.parse();
    val::BaseValue::PointerType node = env.request_value("?foo");
    EXPECT_EQ(node->to_string(), "true");
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
    dip::ValueNode::ListType nodes = env.request_group("?foo.");
    EXPECT_EQ(nodes.at(0)->to_string(), "true");
    EXPECT_EQ(nodes.at(1)->to_string(), "3");
    nodes = env.request_group("?foo.", dip::RequestType::Reference, {"crackle"});
    EXPECT_EQ(nodes.size(), 2);
    EXPECT_EQ(nodes.at(0)->path.name, "bar");
    EXPECT_EQ(nodes.at(1)->path.name, "baz");
    nodes = env.request_group("?foo.", dip::RequestType::Reference, {"snap"});
    EXPECT_EQ(nodes.size(), 1);
    EXPECT_EQ(nodes.at(0)->path.name, "bar");
    nodes = env.request_group("?foo.", dip::RequestType::Reference, {"pop"});
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
    std::vector<dip::ValueNode::ListType> list = env.request_list("?jerk.snap");
    EXPECT_EQ(list.size(), 2);
    {
        const dip::ValueNode::ListType& nodes = list.at(0);
        EXPECT_EQ(nodes.at(0)->path.name, "foo");
    }
    {
        const dip::ValueNode::ListType& nodes = list.at(1);
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
    d.add_string("foo\n  bar bool = true\n  baz int = 3\n");
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

TEST(Environment, GetNode) {
    dip::DIP d;
    d.add_string("foo\n  baz int = 3 cm\n");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 1);
    dip::ValueNode::PointerType node = env.get_node("foo.baz");
    EXPECT_EQ(node->path.name, "foo.baz");
    EXPECT_EQ(node->value->to_string(), "3");
    EXPECT_EQ(node->units->to_string(), "cm");
}

TEST(Environment, RequestFunctionValue) {
    dip::DIP d;
    d.add_function_value("distance", [](const dip::Environment&) {
        return dip::ValueNodeData{std::make_unique<val::ArrayValueFloat64>(2.0), puq::Quantity("m")};
    });
    d.add_string("anchor bool = true");
    dip::Environment env = d.parse();

    dip::ValueNodeData data = env.request_node_data("distance", dip::RequestType::Function);
    ASSERT_TRUE(data.value);
    ASSERT_TRUE(data.units);
    EXPECT_EQ(data.value->to_string(), "2");
    EXPECT_EQ(data.units->to_string(), "m");

    auto value = env.request_value("distance", dip::RequestType::Function);
    ASSERT_TRUE(value);
    EXPECT_EQ(value->to_string(), "2");
    value = env.request_value("distance", dip::RequestType::Function, "cm");
    ASSERT_TRUE(value);
    EXPECT_EQ(value->to_string(), "200");
}

TEST(Environment, RequestFunctionGroup) {
    dip::DIP d;
    d.add_function_nodes("generated", [](const dip::Environment&) {
        dip::ValueNode::ListType nodes;
        nodes.push_back(std::make_shared<dip::IntegerNode>(
            dip::Path("count"), std::make_unique<val::ArrayValueInt32>(3)
        ));
        return nodes;
    });
    d.add_string("anchor bool = true");
    dip::Environment env = d.parse();

    auto nodes = env.request_group("generated", dip::RequestType::Function);
    ASSERT_EQ(nodes.size(), 1);
    EXPECT_EQ(nodes[0]->path.name, "count");
    ASSERT_TRUE(nodes[0]->value);
    EXPECT_EQ(nodes[0]->value->to_string(), "3");
    EXPECT_EQ(nodes[0]->value_origin, dip::ValueOrigin::FunctionRes);
    EXPECT_EQ(nodes[0]->line.source.name, "generated()");
}

TEST(Environment, SelectTagFilter) {
    EXPECT_TRUE((dip::TagFilter{}).matches({}));
    EXPECT_TRUE((dip::TagFilter{{"export", "runtime"}, {"hydro", "gravity"}, {"internal"}})
                    .matches({"export", "runtime", "hydro"}));
    EXPECT_FALSE((dip::TagFilter{{"export", "runtime"}, {}, {}}).matches({"export"}));
    EXPECT_FALSE((dip::TagFilter{{}, {"hydro"}, {}}).matches({}));
    EXPECT_FALSE((dip::TagFilter{{}, {}, {"internal"}}).matches({"export", "internal"}));
    EXPECT_TRUE((dip::TagFilter{{}, {}, {"internal"}}).matches({}));
}

TEST(Environment, SelectSnapshots) {
    dip::DIP parser;
    parser.add_string(
        "physics int = 1\n"
        "  !tags [\"export\"]\n"
        "  speed int = 2 m/s\n"
        "    !tags [\"export\", \"runtime\"]\n"
        "    ?descr \"Flow speed\"\n"
        "  samples[]\n"
        "    value int = 3\n"
        "physics_extra int = 4\n"
    );
    auto env = parser.parse();
    dip::Inspector view{env};
    const auto all = view.select();
    ASSERT_EQ(all.size(), 4);
    const auto subtree = view.select("?physics.");
    ASSERT_EQ(subtree.size(), 3);
    EXPECT_EQ(subtree[0]->path.name, "physics");
    EXPECT_EQ(subtree[1]->path.name, "physics.speed");
    EXPECT_EQ(subtree[2]->path.name, "physics.samples[0].value");
    EXPECT_EQ(view.select("?physics.samples.").size(), 1);
    EXPECT_TRUE(view.select("?missing").empty());
    auto selected = view.select("?physics.", {{"export", "runtime"}, {}, {}});
    ASSERT_EQ(selected.size(), 1);
    auto snapshot = selected.front();
    auto original = env.nodes.at(1);
    EXPECT_NE(snapshot.get(), original.get());
    EXPECT_NE(snapshot->value.get(), original->value.get());
    EXPECT_EQ(snapshot->path.name, "physics.speed");
    EXPECT_EQ(snapshot->metadata.description, "Flow speed");
    // Mutations in either direction must not cross the snapshot boundary.
    snapshot->tags.clear();
    snapshot->metadata.description = "Snapshot only";
    snapshot->set_value(all[0]->value->clone());
    EXPECT_EQ(original->tags.size(), 2);
    EXPECT_EQ(original->metadata.description, "Flow speed");
    EXPECT_EQ(original->value->to_string(), "2");
    original->metadata.description = "Environment only";
    EXPECT_EQ(snapshot->metadata.description, "Snapshot only");
    EXPECT_EQ(env.request_group("?physics.speed").front()->path.name, "speed");
}
