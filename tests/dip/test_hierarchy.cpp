#include "pch_tests.h"

#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>

using namespace snt;

TEST(Hierarchy, ParentChildNodes) {

    dip::DIP d;
    d.add_string("foo int = 2\n  bar int = 3\n  bim int = 4\nbom int = 5");
    dip::Environment env = d.parse();

    dip::BaseNode::PointerType node = env.nodes.at(0);
    EXPECT_EQ(node->path.name, "foo");

    node = env.nodes.at(1);
    EXPECT_EQ(node->path.name, "foo.bar");

    node = env.nodes.at(2);
    EXPECT_EQ(node->path.name, "foo.bim");

    node = env.nodes.at(3);
    EXPECT_EQ(node->path.name, "bom");
}

TEST(Hierarchy, GroupNodes) {

    dip::DIP d;
    d.add_string(
        "foo  \n" // should also accept the empty space after the path
        "  bar int = 3\n"
    );
    dip::Environment env = d.parse();

    dip::BaseNode::PointerType node = env.nodes.at(0);
    EXPECT_EQ(node->path.name, "foo.bar");
}

TEST(Hierarchy, IndentationChecking) {

    dip::DIP d;
    d.add_string(
        "foo int = 2\n"
        " bar int = 3"
    );
    try {
        d.parse();
        FAIL() << "Expected dip::SyntaxException";
    } catch (const dip::SyntaxException& e) {
        EXPECT_EQ(e.info().message, "Invalid indent length");
        EXPECT_EQ(e.info().details, "The indentation length is 1, which is not a multiple of 2.");
        EXPECT_EQ(e.info().suggestion, "Use an indentation length of 0, 2, ...");
    } catch (...) {
        FAIL() << "Expected dip::SyntaxException";
    }

    d = dip::DIP();
    d.add_string(
        "foo int = 2\n"
        "    bar int = 3"
    );
    try {
        d.parse();
        FAIL() << "Expected dip::SyntaxException";
    } catch (const dip::SyntaxException& e) {
        EXPECT_EQ(e.info().message, "Child node has an invalid indent");
        EXPECT_EQ(e.info().details, "The child node is indented 4 spaces, but it should be 2 spaces.");
        EXPECT_EQ(e.info().suggestion, "Indent the child node 2 spaces more than the preceding node.");
    } catch (...) {
        FAIL() << "Expected dip::SyntaxException";
    }
}

TEST(Hierarchy, UnknownCollectionSuggestsNearbyPath) {
    dip::HierarchyList hierarchy;
    hierarchy.set_collection("netcdf.attribute.coordinates", dip::Path::Kind::Group, {});
    try {
        (void)hierarchy.get_collection("netcdf.attribute.cooridnates");
        FAIL() << "Expected dip::EnvironmentException";
    } catch (const dip::EnvironmentException& error) {
        EXPECT_EQ(error.info().message, "Unknown collection");
        EXPECT_EQ(error.info().suggestion, "Did you mean `netcdf.attribute.coordinates`?");
    }

    try {
        (void)hierarchy.get_collection("netcdf.attribute.cxxrdinatxs");
        FAIL() << "Expected dip::EnvironmentException";
    } catch (const dip::EnvironmentException& error) {
        EXPECT_EQ(error.info().suggestion, "Did you mean `netcdf.attribute.coordinates`?");
    }

    try {
        (void)hierarchy.get_collection("unrelated.path");
        FAIL() << "Expected dip::EnvironmentException";
    } catch (const dip::EnvironmentException& error) {
        EXPECT_EQ(error.info().suggestion, "Check whether the collection path is correct.");
    }
}
