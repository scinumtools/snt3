#include "pch_tests.h"

#include <filesystem>
#include <fstream>
#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>

using namespace snt;

TEST(References, BooleanValues) {

    // referencing scalar and array values
    dip::DIP d;
    d.add_string("foo bool = true");
    d.add_string("bar bool = {?foo}");
    d.add_string("snap bool[2] = [true, false]");
    d.add_string("crackle bool[2] = {?snap}");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 4);

    dip::ValueNode::PointerType vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->path.name, "bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "true");

    vnode = env.nodes.at(3);
    EXPECT_EQ(vnode->path.name, "crackle");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[true, false]");
}

TEST(References, IntegerValues) {

    // referencing scalar and array values
    dip::DIP d;
    d.add_string("foo int = 32");
    d.add_string("bar int = {?foo}");
    d.add_string("snap int[2] = [32, 54]");
    d.add_string("crackle int[2] = {?snap}");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 4);

    dip::ValueNode::PointerType vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->path.name, "bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "32");

    vnode = env.nodes.at(3);
    EXPECT_EQ(vnode->path.name, "crackle");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[32, 54]");
}

TEST(References, FloatValues) {

    // referencing scalar and array values
    dip::DIP d;
    d.add_string("foo float = 2.345e6");
    d.add_string("bar float = {?foo}");
    d.add_string("snap float[2] = [2.345e6, 3.456e7]");
    d.add_string("crackle float[2] = {?snap}");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 4);

    dip::ValueNode::PointerType vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->path.name, "bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "2.345e6");

    vnode = env.nodes.at(3);
    EXPECT_EQ(vnode->path.name, "crackle");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[2.345e6, 3.456e7]");
}

TEST(References, StringValues) {

    // referencing scalar and array values
    dip::DIP d;
    d.add_string("foo str = \"baz\"");
    d.add_string("bar str = {?foo}");
    d.add_string("snap str[2] = [\"pop\",\"jerk\"]");
    d.add_string("crackle str[2] = {?snap}");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 4);

    dip::ValueNode::PointerType vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->path.name, "bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "\"baz\"");

    vnode = env.nodes.at(3);
    EXPECT_EQ(vnode->path.name, "crackle");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[\"pop\", \"jerk\"]");
}

TEST(References, ExceptionMissingNode) {

    dip::DIP d;
    d.add_string("bar str = {?foo}");
    try {
        d.parse();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "Value environment request returns an empty pointer: ?foo");
    } catch (...) {
        FAIL() << "Expected std::runtime_error";
    }
}

TEST(References, DataTypeConversion) {

    dip::DIP d;
    d.add_string("foo bool = true");
    d.add_string("bar str = {?foo}");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 2);

    dip::ValueNode::PointerType vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->path.name, "bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "\"true\"");
}

TEST(References, ExceptionDimension) {

    dip::DIP d;
    d.add_string("foo int[2] = [1,2]");
    d.add_string("bar int = {?foo}");
    try {
        d.parse();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "Assigning array value to the scalar node: bar int = {?foo}");
    } catch (...) {
        FAIL() << "Expected std::runtime_error";
    }
}

TEST(References, TableNodes) {

    // referencing scalar and array values
    dip::DIP d;
    d.add_string(
        "foo\n"
        "  snap str = \"snap\"\n"
        "  crackle bool[2] = [true,false]\n"
        "bar table = {?foo}\n"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 4);

    dip::ValueNode::PointerType vnode = env.nodes.at(2);
    EXPECT_EQ(vnode->path.name, "bar.snap");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "\"snap\"");

    vnode = env.nodes.at(3);
    EXPECT_EQ(vnode->path.name, "bar.crackle");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[true, false]");
}

TEST(References, ImportNodes) {

    // referencing scalar and array values
    dip::DIP d;
    d.add_string("foo");
    d.add_string("  snap str = \"snap\"");
    d.add_string("  crackle bool[2] = [true,false]");
    d.add_string("bar {?}");
    d.add_string("baz");
    d.add_string("  {?foo}");
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

TEST(References, RemoteSource) {

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

    // referencing scalar and array values
    dip::DIP d;
    d.add_string("$source " + source_name + " = \"" + source_filename.string() + "\"");
    d.add_string("snap int = {" + source_name + "?foo.bar}");
    d.add_string("crackle bool[2] = {" + source_name + "?foo.baz}");
    d.add_string("pop {" + source_name + "?foo}");
    d.add_string("jerk {" + source_name + "?}");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 6);

    // remove temporary file
    std::filesystem::remove(source_filename);

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->path.name, "snap");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "3");

    vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->path.name, "crackle");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[false, true]");

    vnode = env.nodes.at(2);
    EXPECT_EQ(vnode->path.name, "pop.bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "3");

    vnode = env.nodes.at(3);
    EXPECT_EQ(vnode->path.name, "pop.baz");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[false, true]");

    vnode = env.nodes.at(4);
    EXPECT_EQ(vnode->path.name, "jerk.foo.bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "3");

    vnode = env.nodes.at(5);
    EXPECT_EQ(vnode->path.name, "jerk.foo.baz");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "[false, true]");
}

TEST(References, ExceptionSource) {

    dip::DIP d;
    d.add_string("foo int = {bar?baz}");
    try {
        d.parse();
        FAIL() << "Expected dip::EnvironmentException";
    } catch (const dip::EnvironmentException& e) {
        EXPECT_EQ(e.info().message, "Unknown source");
        EXPECT_EQ(e.info().expected, "The requested source must exist in the environment source list.");
        EXPECT_EQ(e.info().actual, "The source `bar` was not found in the environment source list.");
        EXPECT_EQ(e.info().suggestion, "Check whether the source name is correct.");
    } catch (...) {
        FAIL() << "Expected dip::EnvironmentException";
    }
}

TEST(References, MatchCollectionPath) {

    // referencing scalar and array values
    dip::DIP d;
    d.add_string(
        "foo[bar]\n"
        "  baz bool = false\n"
        "  pop[]\n"
        "    crackle int = 3\n"
        "snap int = {?foo[bar].pop[0].crackle}\n"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 3);

    dip::ValueNode::PointerType vnode = env.nodes.at(2);
    EXPECT_EQ(vnode->path.name, "snap");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "3");
}

TEST(References, RelativePath) {

    dip::DIP d;
    d.add_string(
        "pebble str = \"dupl\"\n"
        "foo\n"
        "  crackle float = 3\n"
        "  kuku str = {..pebble}\n"
        "  bar\n"
        "    pop bool = true\n"
        "    jerk bool = {.pop}\n"
        "    snap float = {..crackle}\n"
    );

    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 6);

    dip::ValueNode::PointerType vnode = env.nodes.at(2);
    EXPECT_EQ(vnode->path.name, "foo.kuku");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "\"dupl\"");

    vnode = env.nodes.at(4);
    EXPECT_EQ(vnode->path.name, "foo.bar.jerk");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "true");

    vnode = env.nodes.at(5);
    EXPECT_EQ(vnode->path.name, "foo.bar.snap");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "3");

    // Throw an error if trying to reference beyond root nodes

    d = dip::DIP();
    d.add_string(
        "crackle int = 3\n"
        "pop\n"
        "  snap int = {...crackle}\n"
    );
    try {
        d.parse();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "Relative path wants to access parents beyong root node: ...crackle");
    } catch (...) {
        FAIL() << "Expected std::runtime_error";
    }
}
