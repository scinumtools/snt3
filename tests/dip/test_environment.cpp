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

// TODO: tests more requests with RequestType::Function

TEST(Environment, RequestCursorCollections) {

    dip::DIP d;
    d.add_string(
        "jerk.snap[crackle]\n"
        "  bar str = \"pot\"\n"
        "  pop[]\n"
        "    foo bool = true\n"
        "  pop[]\n"
        "    foo int = 3\n"
        "  pop[]\n"
        "    foo float = 4e5\n"
        "jerk.snap[lock]\n"
        "  bar str = \"shot\"\n"
        "jerk.baz bool = false\n"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 6);
    dip::Cursor cursor = env.request_cursor();

    // traverse parameter tree and save collection paths and values
    std::vector<std::string> params_parsed;
    params_parsed.reserve(10);
    dip::Cursor group = cursor.get_group("jerk.baz");
    params_parsed.push_back(group.to_string());
    const std::unordered_map<std::string, dip::Cursor> map = cursor.get_map("jerk.snap");
    for (auto [key, item] : map) {
        params_parsed.push_back(item.get_path());
        dip::Cursor group = item.get_group("bar");
        if (key == "crackle") {
            const std::vector<dip::Cursor> list = item.get_list("pop");
            for (auto item : list) {
                params_parsed.push_back(item.get_path());
                dip::Cursor group = item.get_group("foo");
                params_parsed.push_back(group.to_string());
            }
        } else if (key == "lock") {
            dip::Cursor group = item.get_group("bar");
            params_parsed.push_back(group.to_string());
        }
    }
    std::sort(params_parsed.begin(), params_parsed.end());

    // prepare reference parameter set
    std::vector<std::string> params_ref = {
        "jerk.baz = false",
        "jerk.snap[crackle]",
        "jerk.snap[crackle].pop[0]",
        "jerk.snap[crackle].pop[0].foo = true",
        "jerk.snap[crackle].pop[1]",
        "jerk.snap[crackle].pop[1].foo = 3",
        "jerk.snap[crackle].pop[2]",
        "jerk.snap[crackle].pop[2].foo = 4e5",
        "jerk.snap[lock]",
        "jerk.snap[lock].bar = \"shot\"",
    };

    // compare the two parameter sets
    EXPECT_EQ(params_parsed, params_ref);
}

TEST(Environment, RequestCursorValues) {

    dip::DIP d;
    d.add_string(
        "jerk.snap[crackle]\n"
        "  bar str = \"pot\"\n"
        "  pop[]\n"
        "    foo bool = true\n"
        "  pop[]\n"
        "    foo int = 3\n"
        "  pop[]\n"
        "    foo float = 4e5\n"
        "jerk.snap[lock]\n"
        "  bar str = \"shot\"\n"
        "jerk.baz bool = false\n"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 6);
    dip::Cursor cursor = env.request_cursor();

    // test values directly from groups
    {
        dip::Cursor group = cursor.get_group("jerk.baz");
        std::vector<uint8_t> vector = group.get_vector<uint8_t>();
        EXPECT_EQ(vector.size(), 1);
        EXPECT_EQ(vector[0], false);
        uint8_t scalar = group.get_scalar<uint8_t>();
        EXPECT_EQ(scalar, false);
    }
    {
        dip::Cursor group = cursor.get_group("jerk.snap[crackle].pop[1].foo");
        std::vector<int64_t> vector = group.get_vector<int64_t>();
        EXPECT_EQ(vector.size(), 1);
        EXPECT_EQ(vector[0], 3);
        int64_t scalar = group.get_scalar<int64_t>();
        EXPECT_EQ(scalar, 3);
    }
    {
        dip::Cursor group = cursor.get_group("jerk.snap[crackle].pop[2].foo");
        std::vector<double> vector = group.get_vector<double>();
        EXPECT_EQ(vector.size(), 1);
        EXPECT_EQ(vector[0], 4e5);
        double scalar = group.get_scalar<double>();
        EXPECT_EQ(scalar, 4e5);
    }
    {
        dip::Cursor group = cursor.get_group("jerk.snap[lock].bar");
        std::vector<std::string> vector = group.get_vector<std::string>();
        EXPECT_EQ(vector.size(), 1);
        EXPECT_EQ(vector[0], "shot");
        std::string scalar = group.get_scalar<std::string>();
        EXPECT_EQ(scalar, "shot");
    }
}
