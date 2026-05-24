#include "pch_tests.h"

#include <snt/dip/dip.h>

using namespace snt;

TEST(ExpressionsLogical, NonDimensional) {

    // define scalar
    dip::DIP d;
    d.add_string("foo bool = (true && false)");
    d.add_string("bar bool = (true || false)");
    d.add_string("jerk bool = true");
    d.add_string("snap int = 2");
    d.add_string("crackle int = 3");
    d.add_string("pop bool = ({?jerk} || {?snap} == {?crackle})");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 6);

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->name, "foo");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "false");

    vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->name, "bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "true");

    vnode = env.nodes.at(5);
    EXPECT_EQ(vnode->name, "pop");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "true");
}

TEST(ExpressionsLogical, Dimensional) {
    // compare equal
    dip::DIP d;
    d.add_string("foo int = 23 m");
    d.add_string("bar bool = ({?foo} == 2300 cm)");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 2);
    
    dip::ValueNode::PointerType vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->name, "bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->to_string(), "true");

    // compare not equal
    d = dip::DIP();
    d.add_string("foo int = 25 m");
    d.add_string("bar bool = ({?foo} != 25 cm)");
    env = d.parse();
    EXPECT_EQ(env.nodes.size(), 2);
    
    vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->name, "bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->to_string(), "true");

    // compare not less equal
    d = dip::DIP();
    d.add_string("foo int = 26 cm");
    d.add_string("bar bool = ({?foo} <= 25 m)");
    env = d.parse();
    EXPECT_EQ(env.nodes.size(), 2);
    
    vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->name, "bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->to_string(), "true");

    // compare not greater equal
    d = dip::DIP();
    d.add_string("foo int = 24 m");
    d.add_string("bar bool = ({?foo} >= 25 cm)");
    env = d.parse();
    EXPECT_EQ(env.nodes.size(), 2);
    
    vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->name, "bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->to_string(), "true");

    // compare not less
    d = dip::DIP();
    d.add_string("foo int = 26 cm");
    d.add_string("bar bool = ({?foo} < 25 m)");
    env = d.parse();
    EXPECT_EQ(env.nodes.size(), 2);
    
    vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->name, "bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->to_string(), "true");

    // compare not greater
    d = dip::DIP();
    d.add_string("foo int = 24 m");
    d.add_string("bar bool = ({?foo} > 25 cm)");
    env = d.parse();
    EXPECT_EQ(env.nodes.size(), 2);
    
    vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->name, "bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->to_string(), "true");
}

