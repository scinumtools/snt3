#include "pch_tests.h"

#include <snt/dip/dip.h>

using namespace snt;

TEST(Expressions, NestedParentheses) {

    // define scalar
    dip::DIP d;
    d.add_string("foo float = (( 23e6 erg + ( 4 (kg*m2)/s2 ) ) - 6.2415e18 eV ) J");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 1);

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->name, "foo");
    EXPECT_TRUE(vnode);
    // 1e7 erg == 1 J
    // 6.2415e18 eV == 1 J
    // 1 J == 1 kg*m2/s2
    // foo = 2.3 J + 4 J - 1 J = 3.3 J
    EXPECT_EQ(vnode->to_string(), "5.3 J");
}

TEST(Expressions, MultipleLines) {

    // define scalar
    dip::DIP d;
    d.add_string("foo float = (( 1 + 4 )");
    d.add_string("             - 3 )");
    dip::Environment env = d.parse();
    EXPECT_EQ(env.nodes.size(), 1);

    dip::ValueNode::PointerType vnode = env.nodes.at(0);
    EXPECT_EQ(vnode->name, "foo");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->to_string(), "2");

    /* TODO: Implement expressions with Quantities/Measurements with units and not just values
    d = dip::DIP();
    d.add_string("a bool = true");
    d.add_string("b float = 23.43 cm");
    d.add_string("c bool = (false || ( {?b} == 23.43 cm || ~{?a} )");
    d.add_string("          && {?a} || ~!{?c})");
    env = d.parse();
    EXPECT_EQ(env.nodes.size(), 3);

    vnode = env.nodes.at(2);
    EXPECT_EQ(vnode->name, "c");
    EXPECT_TRUE(vnode);
    EXPECT_EQ(vnode->to_string(), "true");
    */
}
