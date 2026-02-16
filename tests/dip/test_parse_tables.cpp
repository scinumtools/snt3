#include "pch_tests.h"
#include <snt/dip/dip.h>

#include <iostream>

using namespace snt;

TEST(ParseTables, BasicTable) {

  dip::DIP d;
  d.add_string("foo table = \"\"\"bar int");
  d.add_string("baz bool");
  d.add_string("dig str");
  d.add_string("---");
  d.add_string("1 true  'a'");
  d.add_string("2 true  'b'");
  d.add_string("3 false 'c'");
  d.add_string("4 true  'd'");
  d.add_string("\"\"\"");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 3);

  dip::ValueNode::PointerType vnode = env.nodes.at(0);
  EXPECT_EQ(vnode->name, "foo.bar");
  EXPECT_EQ(vnode->value_raw, val::Array::StringType({"1", "2", "3", "4"}));
  EXPECT_EQ(vnode->value_shape, val::Array::ShapeType({4}));
  EXPECT_EQ(vnode->value->to_string(), "[1, 2, 3, 4]");
  EXPECT_EQ(vnode->value->get_dtype(), val::DataType::Integer32);

  vnode = env.nodes.at(1);
  EXPECT_EQ(vnode->name, "foo.baz");
  EXPECT_EQ(vnode->value_raw, val::Array::StringType({"true", "true", "false", "true"}));
  EXPECT_EQ(vnode->value_shape, val::Array::ShapeType({4}));
  EXPECT_EQ(vnode->value->to_string(), "[true, true, false, true]");
  EXPECT_EQ(vnode->value->get_dtype(), val::DataType::Boolean);

  vnode = env.nodes.at(2);
  EXPECT_EQ(vnode->name, "foo.dig");
  EXPECT_EQ(vnode->value_raw, val::Array::StringType({"a", "b", "c", "d"}));
  EXPECT_EQ(vnode->value_shape, val::Array::ShapeType({4}));
  EXPECT_EQ(vnode->value->to_string(), "['a', 'b', 'c', 'd']");
  EXPECT_EQ(vnode->value->get_dtype(), val::DataType::String);
}

TEST(ParseTables, EmptySpaceTrimming) {

  dip::DIP d;
  d.add_string("  foo table = \"\"\"");
  d.add_string("bar int");
  d.add_string("baz bool");
  d.add_string("---");
  d.add_string("1 true   ");
  d.add_string("2   true");
  d.add_string("   3 false");
  d.add_string("\"\"\"");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);

  dip::ValueNode::PointerType vnode = env.nodes.at(0);
  EXPECT_EQ(vnode->name, "foo.bar");
  EXPECT_EQ(vnode->value_raw, val::Array::StringType({"1", "2", "3"}));
  EXPECT_EQ(vnode->value_shape, val::Array::ShapeType({3}));
  EXPECT_EQ(vnode->value->to_string(), "[1, 2, 3]");
  EXPECT_EQ(vnode->value->get_dtype(), val::DataType::Integer32);

  vnode = env.nodes.at(1);
  EXPECT_EQ(vnode->name, "foo.baz");
  EXPECT_EQ(vnode->value_raw, val::Array::StringType({"true", "true", "false"}));
  EXPECT_EQ(vnode->value_shape, val::Array::ShapeType({3}));
  EXPECT_EQ(vnode->value->to_string(), "[true, true, false]");
  EXPECT_EQ(vnode->value->get_dtype(), val::DataType::Boolean);
}

TEST(ParseTables, ExceptionDimensionMismatch) {

  dip::DIP d;
  d.add_string("foo table = \"\"\"");
  d.add_string("bar int[10]");
  d.add_string("baz bool");
  d.add_string("---");
  d.add_string("1 true");
  d.add_string("2 true");
  d.add_string("3 false");
  d.add_string("\"\"\"");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Value dimensions do not correspond to the node dimension ranges: [3] != [10]");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(ParseTables, ExceptionMissingColumn) {

  dip::DIP d;
  d.add_string("foo table = \"\"\"");
  d.add_string("bar int[10]");
  d.add_string("baz bool");
  d.add_string("---");
  d.add_string("1");
  d.add_string("2");
  d.add_string("3");
  d.add_string("\"\"\"");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Delimiter ' ' is required: 1");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}
