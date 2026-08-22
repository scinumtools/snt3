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
        EXPECT_EQ(e.info().expected, "Multiple of 2.");
        EXPECT_EQ(e.info().actual, "1");
        EXPECT_EQ(e.info().suggestion, "0, 2, ...");
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
        EXPECT_EQ(e.info().expected, "2");
        EXPECT_EQ(e.info().actual, "4");
        EXPECT_EQ(e.info().suggestion, "Indent 2 spaces more than the preceding node.");
    } catch (...) {
        FAIL() << "Expected dip::SyntaxException";
    }
}
