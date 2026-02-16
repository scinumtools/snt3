#include "pch_tests.h"
#include <snt/dip/dip.h>

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
  EXPECT_EQ(vnode->name, "foo");

  EXPECT_EQ(vnode->value->to_string(), "3");
  EXPECT_EQ(vnode->value->get_dtype(), val::DataType::Integer32);
}

TEST(Modifications, ModificationNode) {

  dip::DIP d;
  d.add_string("foo int = 2");
  d.add_string("foo = 3");
  dip::Environment env = d.parse();

  dip::ValueNode::PointerType vnode = env.nodes.at(0);
  EXPECT_EQ(vnode->value_raw.at(0), "3");
  EXPECT_EQ(vnode->dtype, dip::NodeDtype::Integer);
  EXPECT_EQ(vnode->indent, 0);
  EXPECT_EQ(vnode->name, "foo");

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

  dip::ValueNode::PointerType vnode = env.nodes.at(0);
  EXPECT_EQ(vnode->value_raw.at(0), "3");
  EXPECT_EQ(vnode->dtype, dip::NodeDtype::Integer);
  EXPECT_EQ(vnode->indent, 0);
  EXPECT_EQ(vnode->name, "foo");

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
