#include "pch_tests.h"

#include <iostream>
#include <limits>
#include <snt/dip/dip.h>
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
    }

    // cannot be none if nonzero values are expected
    try {
        dip::DIP d;
        d.add_string("jerk bool[1,2] = none");
        d.parse();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "Value dimensions do not correspond to the node dimension ranges: [0,0] != [1,2]");
    } catch (...) {
        FAIL() << "Expected std::runtime_error";
    }

    //// test empty arrays
    //{
    //    dip::DIP d;
    //    d.add_string("jerk bool[:] = []");
    //    dip::Environment env = d.parse();
    //
    //    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    //    EXPECT_EQ(vnode->value_raw.size(), 0);
    //    EXPECT_EQ(vnode->value, nullptr);
    //}
}
