#include "pch_tests.h"

#include <iostream>
#include <limits>
#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_float.h>

using namespace snt;

TEST(ParseNone, Definition) {

    dip::DIP d;
    d.add_string("jerk bool = none");
    d.add_string("snap int = none");
    d.add_string("crackle float = none");
    d.add_string("pop str = none");
    dip::Environment env = d.parse();

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->value_raw.size(), 0);
    EXPECT_EQ(vnode->value, nullptr);
    EXPECT_EQ(vnode->to_string(), "none");

    vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->value_raw.size(), 0);
    EXPECT_EQ(vnode->value, nullptr);
    EXPECT_EQ(vnode->to_string(), "none");

    vnode = env.nodes.at(2);
    EXPECT_EQ(vnode->value_raw.size(), 0);
    EXPECT_EQ(vnode->value, nullptr);
    EXPECT_EQ(vnode->to_string(), "none");

    vnode = env.nodes.at(3);
    EXPECT_EQ(vnode->value_raw.size(), 0);
    EXPECT_EQ(vnode->value, nullptr);
    EXPECT_EQ(vnode->to_string(), "none");
}

TEST(ParseNone, Arrays) {

    // test definition of arbitrary long arrays
    {
        dip::DIP d;
        d.add_string("jerk bool[:] = none");
        d.add_string("snap int[:] = none");
        d.add_string("crackle float[:] = none");
        d.add_string("pop str[:] = none");
        dip::Environment env = d.parse();

        dip::ValueNode::PointerType vnode = env.nodes.at(0);
        EXPECT_EQ(vnode->value_raw.size(), 0);
        EXPECT_EQ(vnode->value, nullptr);
        EXPECT_EQ(vnode->to_string(), "none");

        vnode = env.nodes.at(1);
        EXPECT_EQ(vnode->value_raw.size(), 0);
        EXPECT_EQ(vnode->value, nullptr);
        EXPECT_EQ(vnode->to_string(), "none");

        vnode = env.nodes.at(2);
        EXPECT_EQ(vnode->value_raw.size(), 0);
        EXPECT_EQ(vnode->value, nullptr);
        EXPECT_EQ(vnode->to_string(), "none");

        vnode = env.nodes.at(3);
        EXPECT_EQ(vnode->value_raw.size(), 0);
        EXPECT_EQ(vnode->value, nullptr);
        EXPECT_EQ(vnode->to_string(), "none");
    }

    // cannot be none if nonzero values are expected
    try {
        dip::DIP d;
        d.add_string("jerk bool[1,2] = none");
        d.parse();
        FAIL() << "Expected dip::SyntaxException";
    } catch (const dip::SyntaxException& e) {
        EXPECT_EQ(e.info().message, "Dimension range mismatch");
        EXPECT_EQ(
            e.info().details, "The value dimensions `[0,0]` do not correspond to the node dimension ranges `[1,2]`."
        );
        EXPECT_EQ(
            e.info().suggestion,
            "Ensure that each value dimension falls within the corresponding dimension range of the node."
        );
    } catch (...) {
        FAIL() << "Expected dip::SyntaxException";
    }
}
