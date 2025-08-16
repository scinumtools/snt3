#include <gtest/gtest.h>

#include "dip/dip.h"
#include "dip/environment.h"
#include "dip/nodes/nodes.h"

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

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "false");

  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "bar");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "true");

  node = env.nodes.at(5);
  EXPECT_EQ(node->name, "pop");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "true");
}
