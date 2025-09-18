#include "fixtures.h"
#include "pch_tests.h"

using namespace snt;

class Functions : public FixtureFunctions {};

TEST_F(Functions, BooleanValues) {

  // define scalar
  dip::DIP d;
  d.add_value_function("scalar_value", FixtureFunctions::get_scalar_boolean);
  d.add_value_function("array_value", FixtureFunctions::get_array_boolean);
  d.add_string("foo bool = (scalar_value)");
  d.add_string("bar bool[3] = (array_value)");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "true");

  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "bar");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(env.nodes.at(1));
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[false, true, false]");
}

TEST_F(Functions, IntegerValues) {

  // define scalar
  dip::DIP d;
  d.add_value_function("scalar_value", FixtureFunctions::get_scalar_integer);
  d.add_value_function("array_value", FixtureFunctions::get_array_integer);
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

TEST_F(Functions, FloatValues) {

  // define scalar
  dip::DIP d;
  d.add_value_function("scalar_value", FixtureFunctions::get_scalar_double);
  d.add_value_function("array_value", FixtureFunctions::get_array_double);
  d.add_string("foo float = (scalar_value)");
  d.add_string("bar float[3] = (array_value)");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "2.34e5");

  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "bar");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "[2.34e5, 3.45e6, 4.56e7]");
}

TEST_F(Functions, StringValues) {

  // define scalar
  dip::DIP d;
  d.add_value_function("scalar_value", FixtureFunctions::get_scalar_string);
  d.add_value_function("array_value", FixtureFunctions::get_array_string);
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

TEST_F(Functions, DataTypeConversion) {

  // returning incompatible value
  dip::DIP d;
  d.add_value_function("foo", FixtureFunctions::get_scalar_boolean);
  d.add_string("bar str = (foo)");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "bar");
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "'true'");
}

TEST_F(Functions, ExceptionDimension) {

  // returning incompatible value
  dip::DIP d;
  d.add_value_function("foo", FixtureFunctions::get_array_string);
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

TEST_F(Functions, TableNodes) {

  dip::DIP d;
  d.add_node_function("scalar_nodes", FixtureFunctions::get_scalar_nodes);
  d.add_node_function("array_nodes", FixtureFunctions::get_array_nodes);
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
  EXPECT_EQ(vnode->value->to_string(), "2.34e5");

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
  EXPECT_EQ(vnode->value->to_string(), "[2.34e5, 3.45e6, 4.56e7]");

  node = env.nodes.at(7);
  EXPECT_EQ(node->name, "bar.array_str");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "['foo', 'bar', 'baz']");
}

TEST_F(Functions, ImportNodes) {

  dip::DIP d;
  d.add_node_function("scalar_nodes", FixtureFunctions::get_scalar_nodes);
  d.add_node_function("array_nodes", FixtureFunctions::get_array_nodes);
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
  EXPECT_EQ(vnode->value->to_string(), "2.34e5");

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
  EXPECT_EQ(vnode->value->to_string(), "[2.34e5, 3.45e6, 4.56e7]");

  node = env.nodes.at(7);
  EXPECT_EQ(node->name, "bar.array_str");
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->value->to_string(), "['foo', 'bar', 'baz']");
}
