#include "pch_tests.h"
#include <snt/dip/dip.h>

using namespace snt;

TEST(Expressions, Logical) {

  // define scalar
  dip::DIP d;
  d.add_string("foo bool = ('true && false')");
  d.add_string("bar bool = ('true || false')");
  d.add_string("jerk bool = true");
  d.add_string("snap int = 2");
  d.add_string("crackle int = 3");
  d.add_string("pop bool = ('{?jerk} || {?snap} == {?crackle}')");
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

TEST(Expressions, Numerical) {

  // define scalar
  dip::DIP d;
  d.add_string("foo int = ('3 + 4 / sqrt( 4 )')");
  d.add_string("bar float = ('1.23 - log( 0.43 * 3. ) ')");
  d.add_string("jerk float = 4.234");
  d.add_string("snap int = 2");
  d.add_string("pop float = ('{?jerk} + {?snap}')");
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
