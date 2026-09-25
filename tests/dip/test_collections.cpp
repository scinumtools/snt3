#include "pch_tests.h"

#include <snt/dip/cursor.h>
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

TEST(Collections, RejectValueCollectionItems) {
    dip::DIP map;
    map.add_string("items[key] int = 1");
    EXPECT_THROW(map.parse(), dip::SyntaxException);

    dip::DIP list;
    list.add_string("items[] bool = true");
    EXPECT_THROW(list.parse(), dip::SyntaxException);
}

TEST(Collections, IndexedListItemModification) {
    dip::DIP parser;
    parser.add_string(
        "softenings list\n"
        "softenings[]\n"
        "  length float = 1 cm\n"
        "softenings[]\n"
        "  length float = 2 cm\n"
        "softenings[1]\n"
        "  length = 3 cm\n"
        "table[2026]\n"
        "  value int = 1\n"
    );
    const dip::Environment env = parser.parse();
    EXPECT_DOUBLE_EQ(env["softenings[1].length"].as<double>(), 3.0);
    EXPECT_EQ(env.hierarchy.get_collection("table").kind, dip::Path::Kind::Map);
    EXPECT_EQ(env["table[2026].value"].as<int64_t>(), 1);

    dip::DIP out_of_range;
    out_of_range.add_string(
        "softenings[]\n"
        "  length float = 1 cm\n"
        "softenings[2]\n"
        "  length = 3 cm\n"
    );
    EXPECT_THROW(out_of_range.parse(), dip::EnvironmentException);
}

TEST(Collections, IndexedListItemSchemaModification) {
    dip::DIP parser;
    parser.add_string(
        "$schema softening\n"
        "  length float = 0 cm\n"
        "softenings list : softening\n"
        "softenings[]\n"
        "  length = 1 cm\n"
        "softenings[]\n"
        "  length = 2 cm\n"
        "softenings[1]\n"
        "  length = 3 cm\n"
    );
    const dip::Environment env = parser.parse();
    EXPECT_DOUBLE_EQ(env["softenings[1].length"].as<double>(), 3.0);
}

TEST(Collections, Declarations) {

    // declare only
    dip::DIP d;
    d.add_string(
        "foo\n"
        "  bar.baz map\n"
        "snap\n"
        "  crackle.pop list"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 0);
    EXPECT_EQ(env.hierarchy.num_collections(), 4);

    dip::Collection col = env.hierarchy.get_collection("foo.bar.baz");
    EXPECT_EQ(col.kind, dip::Path::Kind::Map);

    col = env.hierarchy.get_collection("snap.crackle.pop");
    EXPECT_EQ(col.kind, dip::Path::Kind::List);

    // declare and added items
    d = dip::DIP();
    d.add_string(
        "foo\n"
        "  bar map\n"
        "  bar[baz]\n"
        "    mut int = 3\n"
        "snap\n"
        "  crackle list\n"
        "  crackle[]\n"
        "    pow float = 1.23\n"
    );
    env = d.parse();
    EXPECT_EQ(env.nodes.size(), 2);
    EXPECT_EQ(env.hierarchy.num_collections(), 8);

    col = env.hierarchy.get_collection("foo.bar");
    EXPECT_EQ(col.items.size(), 1);
    EXPECT_EQ(col.kind, dip::Path::Kind::Map);

    col = env.hierarchy.get_collection("snap.crackle");
    EXPECT_EQ(col.items.size(), 1);
    EXPECT_EQ(col.kind, dip::Path::Kind::List);
}
