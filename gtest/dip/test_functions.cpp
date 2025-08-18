#include "dip/dip.h"
#include "dip/environment.h"
#include "dip/nodes/nodes.h"

#include <gtest/gtest.h>

// scalar values
val::BaseValue::PointerType get_scalar_boolean(const dip::Environment& env) {
  return std::make_unique<val::ArrayValue<bool>>(false);
}
val::BaseValue::PointerType get_scalar_integer(const dip::Environment& env) {
  return std::make_unique<val::ArrayValue<int32_t>>(2);
}
val::BaseValue::PointerType get_scalar_double(const dip::Environment& env) {
  return std::make_unique<val::ArrayValue<double>>(2.34e5);
}
val::BaseValue::PointerType get_scalar_string(const dip::Environment& env) {
  return std::make_unique<val::ArrayValue<std::string>>("string");
}

// array values
val::BaseValue::PointerType get_array_boolean(const dip::Environment& env) {
  return val::create_array_value<bool>({false, true, false});
}
val::BaseValue::PointerType get_array_integer(const dip::Environment& env) {
  return val::create_array_value<int>({2, 3, 4, 5}, {2, 2});
}
val::BaseValue::PointerType get_array_double(const dip::Environment& env) {
  return val::create_array_value<double>({2.34e5, 3.45e6, 4.56e7});
}
val::BaseValue::PointerType get_array_string(const dip::Environment& env) {
  return val::create_array_value<std::string>({"foo", "bar", "baz"});
}

TEST(Functions, BooleanValues) {

  // define scalar
  dip::DIP d;
  d.add_value_function("scalar_value", get_scalar_boolean);
  d.add_value_function("array_value", get_array_boolean);
  d.add_string("foo bool = (scalar_value)");
  d.add_string("bar bool[3] = (array_value)");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "false");

  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "bar");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(env.nodes.at(1));
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[false, true, false]");
}

TEST(Functions, IntegerValues) {

  // define scalar
  dip::DIP d;
  d.add_value_function("scalar_value", get_scalar_integer);
  d.add_value_function("array_value", get_array_integer);
  d.add_string("foo int = (scalar_value)");
  d.add_string("bar int[2,2] = (array_value)");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "2");

  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "bar");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[[2, 3], [4, 5]]");
}

TEST(Functions, FloatValues) {

  // define scalar
  dip::DIP d;
  d.add_value_function("scalar_value", get_scalar_double);
  d.add_value_function("array_value", get_array_double);
  d.add_string("foo float = (scalar_value)");
  d.add_string("bar float[3] = (array_value)");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "2.3400e+05");

  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "bar");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[2.3400e+05, 3.4500e+06, 4.5600e+07]");
}

TEST(Functions, StringValues) {

  // define scalar
  dip::DIP d;
  d.add_value_function("scalar_value", get_scalar_string);
  d.add_value_function("array_value", get_array_string);
  d.add_string("foo str = (scalar_value)");
  d.add_string("bar str[3] = (array_value)");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "'string'");

  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "bar");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "['foo', 'bar', 'baz']");
}

TEST(Functions, ExceptionDataType) {

  // returning incompatible value
  dip::DIP d;
  d.add_value_function("foo", get_scalar_boolean);
  d.add_string("bar str = (foo)");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Assigning 'bool' value to the 'str' node: bar str = (foo)");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(Functions, ExceptionDimension) {

  // returning incompatible value
  dip::DIP d;
  d.add_value_function("foo", get_array_string);
  d.add_string("bar str = (foo)");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Assigning array value to the scalar node: bar str = (foo)");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

dip::BaseNode::NodeListType get_scalar_nodes(const dip::Environment& env) {
  return {
      dip::create_scalar_node<bool>("scalar_bool", false),
      dip::create_scalar_node<int>("scalar_int", 1),
      dip::create_scalar_node<double>("scalar_double", 2.34e5),
      dip::create_scalar_node<std::string>("scalar_str", "baz_value"),
  };
}

dip::BaseNode::NodeListType get_array_nodes(const dip::Environment& env) {
  return {
      dip::create_array_node<bool>("array_bool", {false, true, false}),
      dip::create_array_node<int>("array_int", {1, 2, 3, 4}, {2, 2}),
      dip::create_array_node<double>("array_double", {2.34e5, 3.45e6, 4.56e7}),
      dip::create_array_node<std::string>("array_str", {"foo", "bar", "baz"}),
  };
}

TEST(Functions, TableNodes) {

  dip::DIP d;
  d.add_node_function("scalar_nodes", get_scalar_nodes);
  d.add_node_function("array_nodes", get_array_nodes);
  d.add_string("foo table = (scalar_nodes)");
  d.add_string("bar table = (array_nodes)");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 8);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo.scalar_bool");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(env.nodes.at(0));
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "false");

  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "foo.scalar_int");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "1");

  node = env.nodes.at(2);
  EXPECT_EQ(node->name, "foo.scalar_double");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "2.3400e+05");

  node = env.nodes.at(3);
  EXPECT_EQ(node->name, "foo.scalar_str");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "'baz_value'");

  node = env.nodes.at(4);
  EXPECT_EQ(node->name, "bar.array_bool");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[false, true, false]");

  node = env.nodes.at(5);
  EXPECT_EQ(node->name, "bar.array_int");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[[1, 2], [3, 4]]");

  node = env.nodes.at(6);
  EXPECT_EQ(node->name, "bar.array_double");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[2.3400e+05, 3.4500e+06, 4.5600e+07]");

  node = env.nodes.at(7);
  EXPECT_EQ(node->name, "bar.array_str");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "['foo', 'bar', 'baz']");
}

TEST(Functions, ImportNodes) {

  dip::DIP d;
  d.add_node_function("scalar_nodes", get_scalar_nodes);
  d.add_node_function("array_nodes", get_array_nodes);
  d.add_string("foo");
  d.add_string("  (scalar_nodes)");
  d.add_string("bar (array_nodes)");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 8);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo.scalar_bool");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(env.nodes.at(0));
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "false");

  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "foo.scalar_int");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "1");

  node = env.nodes.at(2);
  EXPECT_EQ(node->name, "foo.scalar_double");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "2.3400e+05");

  node = env.nodes.at(3);
  EXPECT_EQ(node->name, "foo.scalar_str");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "'baz_value'");

  node = env.nodes.at(4);
  EXPECT_EQ(node->name, "bar.array_bool");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[false, true, false]");

  node = env.nodes.at(5);
  EXPECT_EQ(node->name, "bar.array_int");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[[1, 2], [3, 4]]");

  node = env.nodes.at(6);
  EXPECT_EQ(node->name, "bar.array_double");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[2.3400e+05, 3.4500e+06, 4.5600e+07]");

  node = env.nodes.at(7);
  EXPECT_EQ(node->name, "bar.array_str");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "['foo', 'bar', 'baz']");
}
