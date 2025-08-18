#include "dip/dip.h"
#include "dip/environment.h"
#include "dip/nodes/nodes.h"

#include <gtest/gtest.h>

TEST(Modifications, DataTypeNode) {

  dip::DIP d;
  d.add_string("foo int = 2");
  d.add_string("foo int32 = 3");
  dip::Environment env = d.parse();

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->value_raw.at(0), "3");
  EXPECT_EQ(node->dtype, dip::NodeDtype::Integer);
  EXPECT_EQ(node->indent, 0);
  EXPECT_EQ(node->name, "foo");

  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(env.nodes.at(0));
  EXPECT_EQ(vnode->value->to_string(), "3");
  EXPECT_EQ(vnode->value->get_dtype(), val::DataType::Integer32);
}

TEST(Modifications, ModificationNode) {

  dip::DIP d;
  d.add_string("foo int = 2");
  d.add_string("foo = 3");
  dip::Environment env = d.parse();

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->value_raw.at(0), "3");
  EXPECT_EQ(node->dtype, dip::NodeDtype::Integer);
  EXPECT_EQ(node->indent, 0);
  EXPECT_EQ(node->name, "foo");

  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(env.nodes.at(0));
  EXPECT_EQ(vnode->value->to_string(), "3");
  EXPECT_EQ(vnode->value->get_dtype(), val::DataType::Integer32);

  // in case modified node was not defined throw an exception
  d = dip::DIP();
  d.add_string("foo = 3");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Modifying undefined node: foo = 3");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(Modifications, Declarations) {

  dip::DIP d;
  d.add_string("foo int");
  d.add_string("foo = 3");
  dip::Environment env = d.parse();

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->value_raw.at(0), "3");
  EXPECT_EQ(node->dtype, dip::NodeDtype::Integer);
  EXPECT_EQ(node->indent, 0);
  EXPECT_EQ(node->name, "foo");

  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(env.nodes.at(0));
  EXPECT_EQ(vnode->value->to_string(), "3");
  EXPECT_EQ(vnode->value->get_dtype(), val::DataType::Integer32);

  // if node is declared but has no value throw an exception
  d = dip::DIP();
  d.add_string("foo int");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Declared node has undefined value: foo int");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}
