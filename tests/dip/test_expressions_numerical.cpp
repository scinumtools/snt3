#include "pch_tests.h"

#include <snt/dip/dip.h>

using namespace snt;

TEST(ExpressionsNumerical, NonDimensional) {

    // define scalar
    dip::DIP d;
    d.add_string("foo int = (3 + 4 / sqrt( 4 ))");
    d.add_string("bar float = (1.23 - log( 0.43 * 3. ))");
    d.add_string("jerk float = 4.234");
    d.add_string("snap int = 2");
    d.add_string("pop float = ({?jerk} + {?snap})");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 5);

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->name, "foo");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "5");

    vnode = env.nodes.at(1);
    EXPECT_EQ(vnode->name, "bar");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "0.9754");

    vnode = env.nodes.at(4);
    EXPECT_EQ(vnode->name, "pop");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->value->to_string(), "6.234");
}

TEST(ExpressionsNumerical, Dimensional) {

    // integer
    dip::DIP d;
    d.add_string("foo int = 23 m");
    d.add_string("bar int = 445 cm");
    d.add_string("baz int = ({?foo} + {?bar}) mm");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 3);

    dip::ValueNode::PointerType vnode = env.nodes.at(2);
    EXPECT_EQ(vnode->name, "baz");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->to_string(), "27450 mm");

    // float: add + subtract
    d = dip::DIP();
    d.add_string("foo float = 23.3 cm");
    d.add_string("bar float = 425.6 mm");
    d.add_string("baz float = ({?foo} + {?bar} - 0.1 m) dm");

    env = d.parse();
    EXPECT_EQ(env.nodes.size(), 3);

    vnode = env.nodes.at(2);
    EXPECT_EQ(vnode->name, "baz");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->to_string(), "5.586 dm");

    // float: multiply + divide
    d = dip::DIP();
    d.add_string("foo float = 23.3 cm");
    d.add_string("bar float = 425.6 mm");
    d.add_string("baz float = ({?foo} / {?bar} * 2 dm) m");

    env = d.parse();
    EXPECT_EQ(env.nodes.size(), 3);

    vnode = env.nodes.at(2);
    EXPECT_EQ(vnode->name, "baz");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->to_string(), "0.1095 m"); // 0.233*m / 0.4256*m * 0.2*m = 0.109492... m
}
