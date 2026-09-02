#include "pch_tests.h"

#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>

using namespace snt;

TEST(Modifications, DataTypeNode) {

    dip::DIP d;
    d.add_string("foo int = 2");
    d.add_string("foo int32 = 3");
    dip::Environment env = d.parse();

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->value_raw.at(0), "3");
    EXPECT_EQ(vnode->dtype, dip::NodeDtype::Integer);
    EXPECT_EQ(vnode->indent, 0);
    EXPECT_EQ(vnode->path.name, "foo");

    EXPECT_EQ(vnode->value->to_string(), "3");
    EXPECT_EQ(vnode->value->get_dtype(), core::DataType::Integer32);
}

TEST(Modifications, ModificationNode) {
    {
        dip::DIP d;
        d.add_string("foo int = 2");
        d.add_string("foo = 3");
        dip::Environment env = d.parse();

        dip::ValueNode::PointerType vnode = env.nodes.at(0);
        EXPECT_EQ(vnode->value_raw.at(0), "3");
        EXPECT_EQ(vnode->dtype, dip::NodeDtype::Integer);
        EXPECT_EQ(vnode->indent, 0);
        EXPECT_EQ(vnode->path.name, "foo");

        EXPECT_EQ(vnode->value->to_string(), "3");
        EXPECT_EQ(vnode->value->get_dtype(), core::DataType::Integer32);
    }
    { // in case modified node was not defined throw an exception
        dip::DIP d;
        d.add_string("foo = 3");
        try {
            d.parse();
            FAIL() << "Expected dip::SyntaxException";
        } catch (const dip::SyntaxException& e) {
            EXPECT_EQ(e.info().message, "Modifying undefined node");
            EXPECT_EQ(
                e.info().details, "The node type has not been defined and no previous node definition was found."
            );
            EXPECT_EQ(
                e.info().suggestion, "Specify a type such as `bool` or `float`, or add a declaration before this node."
            );
        } catch (...) {
            FAIL() << "Expected dip::SyntaxException";
        }
    }
}

TEST(Modifications, Declarations) {

    dip::DIP d;
    d.add_string("foo int");
    d.add_string("foo = 3");
    dip::Environment env = d.parse();

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->value_raw.at(0), "3");
    EXPECT_EQ(vnode->dtype, dip::NodeDtype::Integer);
    EXPECT_EQ(vnode->indent, 0);
    EXPECT_EQ(vnode->path.name, "foo");

    EXPECT_EQ(vnode->value->to_string(), "3");
    EXPECT_EQ(vnode->value->get_dtype(), core::DataType::Integer32);

    // if node is declared but has no value throw an exception
    d = dip::DIP();
    d.add_string("foo int");
    try {
        d.parse();
        FAIL() << "Expected dip::SyntaxException";
    } catch (const dip::SyntaxException& e) {
        EXPECT_EQ(e.info().message, "Undefined value");
        EXPECT_EQ(e.info().details, "The node has a value origin but no value has been defined.");
        EXPECT_EQ(e.info().suggestion, "Provide a valid value for the declared node.");
    } catch (...) {
        FAIL() << "Expected dip::SyntaxException";
    }
}

TEST(Modifications, Reference) {

    dip::DIP d;
    d.add_string(
        "foo int = 3\n"
        "bar int = 1\n"
        "bar = {?foo}"
    );
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 2);

    dip::ValueNode::PointerType vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->path.name, "bar");
    EXPECT_EQ(vnode->to_string(), "3");
}
