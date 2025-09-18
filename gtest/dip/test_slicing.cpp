#include "pch_tests.h"

#include <filesystem>
#include <fstream>

using namespace snt;

TEST(ValueSlicing, FixedRanges) {

  dip::DIP d;
  d.add_string("snap int[2,3] = [[1,2,3], [4,5,6]]");
  d.add_string("crackle int[2] = {?snap}[1,1:2]");
  d.add_string("pop int[2,2] = {?snap}[0:1,0:1]");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 3);

  dip::BaseNode::PointerType node = env.nodes.at(1);
  EXPECT_EQ(node->name, "crackle");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[5, 6]");

  node = env.nodes.at(2);
  EXPECT_EQ(node->name, "pop");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[[1, 2], [4, 5]]");
}

TEST(ValueSlicing, OpenRangesBoolean) {

  dip::DIP d;
  d.add_string("snap bool[2,3] = [[true,false,true], [false,true,false]]");
  d.add_string("crackle bool[3] = {?snap}[0,:]");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);

  dip::BaseNode::PointerType node = env.nodes.at(1);
  EXPECT_EQ(node->name, "crackle");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[true, false, true]");
}

TEST(ValueSlicing, OpenRangesInteger) {

  dip::DIP d;
  d.add_string("snap int[2,3] = [[1,2,3], [4,5,6]]");
  d.add_string("crackle int[3] = {?snap}[0,:]");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);

  dip::BaseNode::PointerType node = env.nodes.at(1);
  EXPECT_EQ(node->name, "crackle");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[1, 2, 3]");
}

TEST(ValueSlicing, OpenRangesFloat) {

  dip::DIP d;
  d.add_string("snap float[2,3] = [[1,2,3], [4,5,6]]");
  d.add_string("crackle float[3] = {?snap}[0,:]");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);

  dip::BaseNode::PointerType node = env.nodes.at(1);
  EXPECT_EQ(node->name, "crackle");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[1, 2, 3]");
}

TEST(ValueSlicing, OpenRangesString) {

  dip::DIP d;
  d.add_string("snap str[2,3] = [[a,b,c], [d,e,f]]");
  d.add_string("crackle str[3] = {?snap}[0,:]");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);

  dip::BaseNode::PointerType node = env.nodes.at(1);
  EXPECT_EQ(node->name, "crackle");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "['a', 'b', 'c']");
}

TEST(ValueSlicing, ArrayToScalar) {

  dip::DIP d;
  d.add_string("snap int[2,3] = [[1,2,3], [4,5,6]]");
  d.add_string("crackle int = {?snap}[1,0]");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);

  dip::BaseNode::PointerType node = env.nodes.at(1);
  EXPECT_EQ(node->name, "crackle");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "4");
}
