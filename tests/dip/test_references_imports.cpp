#include "pch_tests.h"

#include <filesystem>
#include <fstream>
#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>

using namespace snt;

TEST(ReferencesImports, ImportNode) {

    // import a single node
    dip::DIP d;
    d.add_string("foo");
    d.add_string("  snap str = \"snap\"");
    d.add_string("  crackle bool[2] = [true,false]");
    d.add_string("bar {?foo.snap}");
    d.add_string("baz");
    d.add_string("  {?foo.crackle}");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 4);

    dip::ValueNode::PointerType vnode = env.nodes.at(2);
    EXPECT_EQ(vnode->path.name, "bar.snap");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "\"snap\"");

    vnode = env.nodes.at(3);
    EXPECT_EQ(vnode->path.name, "baz.crackle");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[true, false]");
}

TEST(ReferencesImports, ImportNodeRemote) {

    // create temporary file
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path source_filename = temp_dir / "example_source.dip";
    std::string source_code = "foo\n"
                              "  bar int = 3\n";
    std::string source_name = "foo_source";
    {
        std::ofstream source_file(source_filename);
        ASSERT_TRUE(source_file.is_open()) << "Failed to create temp file.";
        source_file << source_code;
    }

    // referencing scalar and array nodes
    dip::DIP d;
    d.add_string("$source " + source_name + " = \"" + source_filename.string() + "\"");
    d.add_string("pop {" + source_name + "?foo.bar}");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 1);

    // remove temporary file
    std::filesystem::remove(source_filename);

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->path.name, "pop.bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "3");
}

TEST(ReferencesImports, ImportRoot) {

    // referencing scalar and array values
    dip::DIP d;
    d.add_string("foo");
    d.add_string("  snap str = \"snap\"");
    d.add_string("  crackle bool[2] = [true,false]");
    d.add_string("bar {?.}");
    d.add_string("baz");
    d.add_string("  {?foo.}");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 6);

    dip::ValueNode::PointerType vnode = env.nodes.at(2);
    EXPECT_EQ(vnode->path.name, "bar.foo.snap");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "\"snap\"");

    vnode = env.nodes.at(3);
    EXPECT_EQ(vnode->path.name, "bar.foo.crackle");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[true, false]");

    vnode = env.nodes.at(4);
    EXPECT_EQ(vnode->path.name, "baz.snap");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "\"snap\"");

    vnode = env.nodes.at(5);
    EXPECT_EQ(vnode->path.name, "baz.crackle");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[true, false]");
}

TEST(ReferencesImports, ImportRootRemote) {

    // create temporary file
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path source_filename = temp_dir / "example_source.dip";
    std::string source_code = "foo\n"
                              "  bar int = 3\n"
                              "  baz bool[2] = [false,true]";
    std::string source_name = "foo_source";
    {
        std::ofstream source_file(source_filename);
        ASSERT_TRUE(source_file.is_open()) << "Failed to create temp file.";
        source_file << source_code;
    }

    // referencing scalar and array nodes
    dip::DIP d;
    d.add_string("$source " + source_name + " = \"" + source_filename.string() + "\"");
    d.add_string("pop {" + source_name + "?foo.}");
    d.add_string("jerk {" + source_name + "?.}");
    d.add_string("snap {" + source_name + "?}");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 6);

    // remove temporary file
    std::filesystem::remove(source_filename);

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->path.name, "pop.bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "3");

    vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->path.name, "pop.baz");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[false, true]");

    vnode = env.nodes.at(2);
    EXPECT_EQ(vnode->path.name, "jerk.foo.bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "3");

    vnode = env.nodes.at(3);
    EXPECT_EQ(vnode->path.name, "jerk.foo.baz");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[false, true]");

    vnode = env.nodes.at(4);
    EXPECT_EQ(vnode->path.name, "snap.foo.bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "3");

    vnode = env.nodes.at(5);
    EXPECT_EQ(vnode->path.name, "snap.foo.baz");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[false, true]");
}
