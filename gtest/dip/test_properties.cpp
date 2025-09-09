#include "pch_tests.h"

#include <iostream>
#include <limits>

using namespace snt;

TEST(Properties, Constant) {

  dip::DIP d;
  d.add_string("foo bool = true");
  d.add_string(" !constant");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1); // constant declaration is not returned as a separate node

  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(env.nodes.at(0));
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->constant, true); // foo node is set as a constant

  // Throw an error if one is trying to override a constant node
  d = dip::DIP();
  d.add_string("foo bool = true");
  d.add_string("  !constant");
  d.add_string("foo bool = false");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Node 'foo' is constant and cannot be modified: foo bool = true");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }

  // Throw an error if indent is not higher
  d = dip::DIP();
  d.add_string("  foo bool = true");
  d.add_string("!constant");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "The indent '0' of a property is not higher than the indent '2' of a preceding node: !constant");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(Properties, Description) {

  dip::DIP d;
  d.add_string("foo bool = true");
  d.add_string("  !descr 'If foo is true, bar is false'");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1); // description is not returned as a separate node

  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(env.nodes.at(0));
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->description, "If foo is true, bar is false");

  // Throw an error if trying to assign description to unsuported node
  // TODO: The test below works only because the group node is the first node in the list.
  //       If there are value nodes before the group node, the description will be assigned
  //       to the first previous value node. This is because description nodes
  //       currently don't see the group nodes in a node list.
  //       This needs to be fixed!
  d = dip::DIP();
  d.add_string("foo");
  d.add_string("  !descr 'This is a group node'");
  d.add_string("bar int = 3");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Only value nodes (bool, int, real and str) can have properties:   !descr 'This is a group node'");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }

  // Throw an error if indent is not higher
  d = dip::DIP();
  d.add_string("  foo bool = true");
  d.add_string("!descr 'If foo is true, bar is false'");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "The indent '0' of a property is not higher than the indent '2' of a preceding node: !descr 'If foo is true, bar is false'");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(Properties, Format) {

  dip::DIP d;
  d.add_string("foo str = 'bar'");
  d.add_string("  !format '[a-z]+'");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1); // format is not returned as a separate node

  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(env.nodes.at(0));
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->format, "[a-z]+");

  // Throw an error if node value does not match expected format
  d = dip::DIP();
  d.add_string("foo str = 'sdf34'");
  d.add_string("  !format '[a-z]+'");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Node value 'sdf34' does not match with expected format '[a-z]+'");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }

  // Throw an error if indent is not higher
  d = dip::DIP();
  d.add_string("  foo str = 'bar'");
  d.add_string("!format '[a-z]+'");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "The indent '0' of a property is not higher than the indent '2' of a preceding node: !format '[a-z]+'");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(Properties, Tags) {

  dip::DIP d;
  d.add_string("foo str = 'bar'");
  d.add_string("  !tags ['baz','word']");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1); // tags is not returned as a separate node

  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(env.nodes.at(0));
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->tags[0], "baz");
  EXPECT_EQ(vnode->tags[1], "word");

  // Throw an error if indent is not higher
  d = dip::DIP();
  d.add_string("  foo str = 'bar'");
  d.add_string("!tags '[a-z]+'");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "The indent '0' of a property is not higher than the indent '2' of a preceding node: !tags '[a-z]+'");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(Properties, Condition) {

  dip::DIP d;
  d.add_string("foo str = 'bar'");
  d.add_string("  !condition true");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1); // condition is not returned as a separate node

  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(env.nodes.at(0));
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->condition, "true");

  // Throw an error if indent is not higher
  d = dip::DIP();
  d.add_string("foo str = 'bar'");
  d.add_string("  !condition false");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Node does not satisfy the given condition: false");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(Properties, OptionsBolean) {

  // boolean nodes cannot have options
  dip::DIP d;
  d.add_string("foo bool = true");
  d.add_string("  !options [false,true]");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Option property is not implemented for boolean nodes: foo bool = true");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(Properties, OptionsInteger) {

  dip::DIP d;
  d.add_string("foo int = 32");
  d.add_string("  !options [16,32,64]");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1); // tags is not returned as a separate node

  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(env.nodes.at(0));
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->options.size(), 3);
  EXPECT_EQ(vnode->options[0].value->to_string(), "16");
  EXPECT_EQ(vnode->options[1].value->to_string(), "32");
  EXPECT_EQ(vnode->options[2].value->to_string(), "64");

  // TODO: implement unit conversion of units

  // validate if node value is in options
  d = dip::DIP();
  d.add_string("foo int = 1");
  d.add_string("  !options [16,32,64]");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Value 1 of node 'foo' doesn't match with any option: 16, 32, 64");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(Properties, OptionsReal) {

  dip::DIP d;
  d.add_string("foo real = 2.34");
  d.add_string("  !options [1,2.34,5.6e7]");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1); // tags is not returned as a separate node

  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(env.nodes.at(0));
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->options.size(), 3);
  EXPECT_EQ(vnode->options[0].value->to_string(), "1");
  EXPECT_EQ(vnode->options[1].value->to_string(), "2.34");
  EXPECT_EQ(vnode->options[2].value->to_string(), "5.6e7");

  // TODO: implement unit conversion of units

  // validate if node value is in options
  d = dip::DIP();
  d.add_string("foo real = 2");
  d.add_string("  !options [1,2.34,5.6e7]");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Value 2 of node 'foo' doesn't match with any option: 1, 2.34, 5.6e7");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(Properties, OptionsString) {

  dip::DIP d;
  d.add_string("foo str = 'bar'");
  d.add_string("  !options ['bar','snap','pow']");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1); // tags is not returned as a separate node
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(env.nodes.at(0));
  EXPECT_TRUE(vnode);
  EXPECT_EQ(vnode->options.size(), 3);
  EXPECT_EQ(vnode->options[0].value->to_string(), "'bar'");
  EXPECT_EQ(vnode->options[1].value->to_string(), "'snap'");
  EXPECT_EQ(vnode->options[2].value->to_string(), "'pow'");

  // validate if node value is in options
  d = dip::DIP();
  d.add_string("foo str = 'baz'");
  d.add_string("  !options ['bar','snap','pow']");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Value 'baz' of node 'foo' doesn't match with any option: 'bar', 'snap', 'pow'");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(Properties, OptionsMultiline) {

  dip::DIP d;
  d.add_string("jerk str = 'snap'");
  d.add_string("  = snap");
  d.add_string("  = crackle");
  d.add_string("  = pop");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 1); // tags is not returned as a separate node
}

TEST(Properties, TableDelimiter) {

  dip::DIP d;
  d.add_string("foo table = \"\"\"bar int");
  d.add_string("baz bool");
  d.add_string("---");
  d.add_string("1, true");
  d.add_string("2, false");
  d.add_string("\"\"\"");
  d.add_string("  !delimiter ,");
  dip::Environment env = d.parse();
  EXPECT_EQ(env.nodes.size(), 2);

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo.bar");
  EXPECT_EQ(node->value_raw, val::Array::StringType({"1", "2"}));
  EXPECT_EQ(node->value_shape, val::Array::ShapeType({2}));
  dip::ValueNode::PointerType vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_EQ(vnode->value->to_string(), "[1, 2]");
  EXPECT_EQ(vnode->value->get_dtype(), val::DataType::Integer32);

  node = env.nodes.at(1);
  EXPECT_EQ(node->name, "foo.baz");
  EXPECT_EQ(node->value_raw, val::Array::StringType({"true", "false"}));
  EXPECT_EQ(node->value_shape, val::Array::ShapeType({2}));
  vnode = std::dynamic_pointer_cast<dip::ValueNode>(node);
  EXPECT_EQ(vnode->value->to_string(), "[true, false]");
  EXPECT_EQ(vnode->value->get_dtype(), val::DataType::Boolean);
}
