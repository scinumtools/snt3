#include "pch_tests.h"

#include <snt/dip/dip.h>

using namespace snt;

TEST(Collections, MapItems) {

    dip::DIP d;
    d.add_string("foo.fee[bar]");
    d.add_string("  baz int = 3");
    d.add_string("  jerk.snap[crackle]");
    d.add_string("    pop int = 4");
    d.add_string("  jerk.snap[jolt]");
    d.add_string("    surge int = 5");
    dip::Environment env = d.parse();

    // test nodes
    EXPECT_EQ(env.nodes.size(), 3);
    dip::BaseNode::PointerType node = env.nodes.at(0);
    EXPECT_EQ(node->path.name, "foo.fee[bar].baz");
    node = env.nodes.at(1);
    EXPECT_EQ(node->path.name, "foo.fee[bar].jerk.snap[crackle].pop");
    node = env.nodes.at(2);
    EXPECT_EQ(node->path.name, "foo.fee[bar].jerk.snap[jolt].surge");

    // test collections
    std::unordered_map<std::string, dip::Collection> collections = env.hierarchy.get_collections();
    EXPECT_EQ(collections.size(), 8);
    {
        auto cl = collections.find("foo.fee");
        EXPECT_NE(cl, collections.end());
        EXPECT_EQ(cl->second.kind, dip::Path::Kind::Map);
        EXPECT_EQ(cl->second.items, std::vector<std::string>({"bar"}));
    }
    {
        auto cl = collections.find("foo.fee[bar].jerk.snap");
        EXPECT_TRUE(cl != collections.end());
        EXPECT_EQ(cl->second.kind, dip::Path::Kind::Map);
        EXPECT_EQ(cl->second.items, std::vector<std::string>({"crackle", "jolt"}));
    }
}

TEST(Collections, ListItems) {

    dip::DIP d;
    d.add_string("foo.fee[]");
    d.add_string("  baz int = 3");
    d.add_string("  jerk.snap[]");
    d.add_string("    pop int = 4");
    d.add_string("  jerk.snap[]");
    d.add_string("    surge int = 5");
    dip::Environment env = d.parse();

    // test nodes
    EXPECT_EQ(env.nodes.size(), 3);
    dip::BaseNode::PointerType node = env.nodes.at(0);
    EXPECT_EQ(node->path.name, "foo.fee[0].baz");
    node = env.nodes.at(1);
    EXPECT_EQ(node->path.name, "foo.fee[0].jerk.snap[0].pop");
    node = env.nodes.at(2);
    EXPECT_EQ(node->path.name, "foo.fee[0].jerk.snap[1].surge");

    // test collections
    std::unordered_map<std::string, dip::Collection> collections = env.hierarchy.get_collections();
    EXPECT_EQ(collections.size(), 8);
    {
        auto cl = collections.find("foo.fee");
        EXPECT_NE(cl, collections.end());
        EXPECT_EQ(cl->second.kind, dip::Path::Kind::List);
        EXPECT_EQ(cl->second.items, std::vector<std::string>({"0"}));
    }
    {
        auto cl = collections.find("foo.fee[0].jerk.snap");
        EXPECT_TRUE(cl != collections.end());
        EXPECT_EQ(cl->second.kind, dip::Path::Kind::List);
        EXPECT_EQ(cl->second.items, std::vector<std::string>({"0", "1"}));
    }
}
