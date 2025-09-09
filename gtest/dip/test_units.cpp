#include "pch_tests.h"

#include <iostream>
#include <limits>

using namespace snt;

TEST(Units, Definition) {

  dip::DIP d;
  d.add_string("foo int = 23 km");
  dip::Environment env = d.parse();

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  EXPECT_EQ(node->units_raw, "km");
  dip::QuantityNode::PointerType qnode = std::dynamic_pointer_cast<dip::QuantityNode>(node);
  EXPECT_TRUE(qnode);
  EXPECT_EQ(qnode->value->to_string(), "23");
  EXPECT_EQ(qnode->units->to_string(), "km");

  d = dip::DIP();
  d.add_string("foo real = 2.34e5 km");
  env = d.parse();

  node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  EXPECT_EQ(node->units_raw, "km");
  qnode = std::dynamic_pointer_cast<dip::QuantityNode>(node);
  EXPECT_TRUE(qnode);
  EXPECT_EQ(qnode->value->to_string(), "2.34e5");
  EXPECT_EQ(qnode->units->to_string(), "km");
}

TEST(Units, ScalarModification) {

  dip::DIP d;
  d.add_string("foo int = 23 m");
  d.add_string("foo = 23 km");
  dip::Environment env = d.parse();

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  EXPECT_EQ(node->units_raw, "m");
  dip::QuantityNode::PointerType qnode = std::dynamic_pointer_cast<dip::QuantityNode>(node);
  EXPECT_TRUE(qnode);
  EXPECT_EQ(qnode->value->to_string(), "23000");
  EXPECT_EQ(qnode->units->to_string(), "m");

  d = dip::DIP();
  d.add_string("foo real = 23 km");
  d.add_string("foo = 23 m");
  env = d.parse();

  node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  EXPECT_EQ(node->units_raw, "km");
  qnode = std::dynamic_pointer_cast<dip::QuantityNode>(node);
  EXPECT_TRUE(qnode);
  EXPECT_EQ(qnode->value->to_string(), "0.023");
  EXPECT_EQ(qnode->units->to_string(), "km");
}

TEST(Units, ArrayModification) {

  dip::DIP d;
  d.add_string("foo int[2] = [23, 45] m");
  d.add_string("foo = [12, 23] km");
  dip::Environment env = d.parse();

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  EXPECT_EQ(node->units_raw, "m");
  dip::QuantityNode::PointerType qnode = std::dynamic_pointer_cast<dip::QuantityNode>(node);
  EXPECT_TRUE(qnode);
  EXPECT_EQ(qnode->value->to_string(), "[12000, 23000]");
  EXPECT_EQ(qnode->units->to_string(), "m");

  d = dip::DIP();
  d.add_string("foo real[2] = [23, 45] m");
  d.add_string("foo = [12, 23] km");
  env = d.parse();

  node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  EXPECT_EQ(node->units_raw, "m");
  qnode = std::dynamic_pointer_cast<dip::QuantityNode>(node);
  EXPECT_TRUE(qnode);
  EXPECT_EQ(qnode->value->to_string(), "[1.2e4, 2.3e4]");
  EXPECT_EQ(qnode->units->to_string(), "m");
}

TEST(Units, DimlessModification) {

  dip::DIP d;
  d.add_string("foo int = 23 km");
  d.add_string("foo = 45");
  dip::Environment env = d.parse();

  dip::BaseNode::PointerType node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  EXPECT_EQ(node->units_raw, "km");
  dip::QuantityNode::PointerType qnode = std::dynamic_pointer_cast<dip::QuantityNode>(node);
  EXPECT_TRUE(qnode);
  EXPECT_EQ(qnode->value->to_string(), "45");
  EXPECT_EQ(qnode->units->to_string(), "km");

  d = dip::DIP();
  d.add_string("foo real = 23 km");
  d.add_string("foo = 45");
  env = d.parse();

  node = env.nodes.at(0);
  EXPECT_EQ(node->name, "foo");
  EXPECT_EQ(node->units_raw, "km");
  qnode = std::dynamic_pointer_cast<dip::QuantityNode>(node);
  EXPECT_TRUE(qnode);
  EXPECT_EQ(qnode->value->to_string(), "45");
  EXPECT_EQ(qnode->units->to_string(), "km");
}

TEST(Units, OptionSetting) {

  dip::DIP d;
  d.add_string("foo int = 2000 m");
  d.add_string("  !options [2, 3] km");
  dip::Environment env = d.parse();

  dip::BaseNode::PointerType node = env.nodes.at(0);
  dip::QuantityNode::PointerType qnode = std::dynamic_pointer_cast<dip::QuantityNode>(node);
  EXPECT_TRUE(qnode);
  EXPECT_EQ(qnode->value->to_string(), "2000");
  EXPECT_EQ(qnode->units->to_string(), "m");
  EXPECT_EQ(qnode->options[0].value->to_string(), "2e3");
  EXPECT_EQ(qnode->options[1].value->to_string(), "3e3");

  d = dip::DIP();
  d.add_string("foo real = 2000 m");
  d.add_string("  !options [2, 3] km");
  env = d.parse();

  node = env.nodes.at(0);
  qnode = std::dynamic_pointer_cast<dip::QuantityNode>(node);
  EXPECT_TRUE(qnode);
  EXPECT_EQ(qnode->value->to_string(), "2e3");
  EXPECT_EQ(qnode->units->to_string(), "m");
  EXPECT_EQ(qnode->options[0].value->to_string(), "2e3");
  EXPECT_EQ(qnode->options[1].value->to_string(), "3e3");
}

TEST(Units, ScalarInjection) {

  dip::DIP d;
  d.add_string("foo int = 2000 m");
  d.add_string("bar int = {?foo} km");
  dip::Environment env = d.parse();

  dip::BaseNode::PointerType node = env.nodes.at(1);
  dip::QuantityNode::PointerType qnode = std::dynamic_pointer_cast<dip::QuantityNode>(node);
  EXPECT_TRUE(qnode);
  EXPECT_EQ(qnode->value->to_string(), "2");
  EXPECT_EQ(qnode->units->to_string(), "km");

  d = dip::DIP();
  d.add_string("foo real = 2000 m");
  d.add_string("bar real = {?foo} km");
  env = d.parse();

  node = env.nodes.at(1);
  qnode = std::dynamic_pointer_cast<dip::QuantityNode>(node);
  EXPECT_TRUE(qnode);
  EXPECT_EQ(qnode->value->to_string(), "2");
  EXPECT_EQ(qnode->units->to_string(), "km");
}

TEST(Units, ArrayInjection) {

  dip::DIP d;
  d.add_string("foo int[2] = [2000, 3000] m");
  d.add_string("bar int[2] = {?foo} km");
  dip::Environment env = d.parse();

  dip::BaseNode::PointerType node = env.nodes.at(1);
  dip::QuantityNode::PointerType qnode = std::dynamic_pointer_cast<dip::QuantityNode>(node);
  EXPECT_TRUE(qnode);
  EXPECT_EQ(qnode->value->to_string(), "[2, 3]");
  EXPECT_EQ(qnode->units->to_string(), "km");

  d = dip::DIP();
  d.add_string("foo real[2] = [2.23e4, 3.45e6] m");
  d.add_string("bar real[2] = {?foo} km");
  env = d.parse();

  node = env.nodes.at(1);
  qnode = std::dynamic_pointer_cast<dip::QuantityNode>(node);
  EXPECT_TRUE(qnode);
  EXPECT_EQ(qnode->value->to_string(), "[22.3, 3.45e3]");
  EXPECT_EQ(qnode->units->to_string(), "km");
}

TEST(Units, DimlessModificationError) {

  dip::DIP d;
  d.add_string("foo real = 23");
  d.add_string("foo = 23 m");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Trying to convert 'm' into a nondimensional quantity: foo real = 23");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(Units, DimlessInjectionError) {

  dip::DIP d;
  d.add_string("foo real = 23 m");
  d.add_string("bar real = {?foo}");
  try {
    d.parse();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Trying to convert 'm' into a nondimensional quantity: foo real = 23 m");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(Units, UnitConversionError) {

  dip::DIP d;
  d.add_string("foo real = 23 km");
  d.add_string("foo = 23 g");
  EXPECT_THROW(d.parse(), puq::ConvDimExcept);
}
