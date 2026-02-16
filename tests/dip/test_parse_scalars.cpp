#include "pch_tests.h"
#include <snt/dip/dip.h>
#include <snt/dip/nodes/node_float.h>

#include <iostream>
#include <limits>

using namespace snt;

TEST(ParseScalars, BooleanValue) {

  dip::DIP d;
  d.add_string("foo1 bool = true");
  d.add_string("foo2 bool = false");
  dip::Environment env = d.parse();

  dip::ValueNode::PointerType vnode = env.nodes.at(0);
  EXPECT_EQ(vnode->value_raw.at(0), "true");
  EXPECT_EQ(vnode->dtype, dip::NodeDtype::Boolean);
  EXPECT_EQ(vnode->indent, 0);
  EXPECT_EQ(vnode->name, "foo1");

  EXPECT_EQ(vnode->value->to_string(), "true");
  EXPECT_EQ(vnode->value->get_dtype(), val::DataType::Boolean);

  vnode = env.nodes.at(1);
  EXPECT_EQ(vnode->value_raw.at(0), "false");
  EXPECT_EQ(vnode->value->to_string(), "false");
}

TEST(ParseScalars, IntegerValue) {

  dip::DIP d;
  d.add_string("foo1 int = 23");
  d.add_string("foo2 int = 23456789");
  dip::Environment env = d.parse();

  dip::ValueNode::PointerType vnode = env.nodes.at(0);
  EXPECT_EQ(vnode->value_raw.at(0), "23");
  EXPECT_EQ(vnode->dtype, dip::NodeDtype::Integer);
  EXPECT_EQ(vnode->indent, 0);
  EXPECT_EQ(vnode->name, "foo1");

  EXPECT_EQ(vnode->value->to_string(), "23");
  EXPECT_EQ(vnode->value->get_dtype(), val::DataType::Integer32);

  vnode = env.nodes.at(1);
  EXPECT_EQ(vnode->value->to_string(), "23456789");
}

TEST(ParseScalars, FloatValue) {

  dip::DIP d;
  d.add_string("foo1 float = 23");
  d.add_string("foo2 float = 23.456");
  d.add_string("foo3 float = 23.456e7");
  d.add_string("foo4 float32 = 23.123456789123456789");
  d.add_string("foo5 float64 = 23.123456789123456789");
  if (dip::FloatNode::max_float_size == 128)
    d.add_string("foo6 float128 = 23.123456789123456789");
  dip::Environment env = d.parse();

  dip::ValueNode::PointerType vnode = env.nodes.at(0);
  EXPECT_EQ(vnode->value_raw.at(0), "23");
  EXPECT_EQ(vnode->dtype, dip::NodeDtype::Float);
  EXPECT_EQ(vnode->indent, 0);
  EXPECT_EQ(vnode->name, "foo1");

  EXPECT_EQ(vnode->value->to_string(), "23");
  EXPECT_EQ(vnode->value->get_dtype(), val::DataType::Float64);

  vnode = env.nodes.at(1);
  EXPECT_EQ(vnode->value_raw.at(0), "23.456");
  EXPECT_EQ(vnode->value->to_string(), "23.46");

  vnode = env.nodes.at(2);
  EXPECT_EQ(vnode->value_raw.at(0), "23.456e7");
  EXPECT_EQ(vnode->value->to_string(), "2.346e8");

  snt::StringFormatType fmt;

  vnode = env.nodes.at(3);
  fmt.valuePrecision = 7;
  EXPECT_EQ(vnode->value->to_string(fmt), "23.12346");

  vnode = env.nodes.at(4);
  fmt.valuePrecision = 13;
  EXPECT_EQ(vnode->value->to_string(fmt), "23.12345678912");

  // TODO: This case needs more testing on other platforms
  if (dip::FloatNode::max_float_size == 128) {
    vnode = env.nodes.at(5);
    std::cout << dip::FloatNode::max_float_size << '\n';
    fmt.valuePrecision = 18;
    EXPECT_EQ(vnode->value->to_string(fmt), "23.1234567891234568");
  }
}

TEST(ParseScalars, StringValue) {

  dip::DIP d;
  d.add_string("foo str = \"bar\"");
  dip::Environment env = d.parse();

  dip::ValueNode::PointerType vnode = env.nodes.at(0);
  EXPECT_EQ(vnode->value_raw.at(0), "bar");
  EXPECT_EQ(vnode->dtype, dip::NodeDtype::String);
  EXPECT_EQ(vnode->indent, 0);
  EXPECT_EQ(vnode->name, "foo");

  EXPECT_EQ(vnode->value->to_string(), "'bar'");
  EXPECT_EQ(vnode->value->get_dtype(), val::DataType::String);
}

TEST(ParseScalars, Cloning) {

  dip::DIP d;
  d.add_string("jerk bool = true");
  d.add_string("snap int = 3 cm");
  d.add_string("crackle float = 1.23 J");
  d.add_string("pop str = 'foo'");
  dip::Environment env = d.parse();

  dip::ValueNode::PointerType clone;

  dip::ValueNode::PointerType jerk = env.nodes.at(0);
  clone = jerk->clone("clone");
  EXPECT_EQ(clone->to_string(), "true");

  dip::ValueNode::PointerType snap = env.nodes.at(1);
  clone = snap->clone("clone");
  EXPECT_EQ(clone->to_string(), "3 cm");

  dip::ValueNode::PointerType crackle = env.nodes.at(2);
  clone = crackle->clone("clone");
  EXPECT_EQ(clone->to_string(), "1.23 J");

  dip::ValueNode::PointerType pop = env.nodes.at(3);
  clone = pop->clone("clone");
  EXPECT_EQ(clone->to_string(), "'foo'");
}
