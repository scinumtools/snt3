#ifndef GTEST_FIXTURES_H
#define GTEST_FIXTURES_H

#include <gtest/gtest.h>

class FixtureFunctions : public ::testing::Test {
protected:
  // scalar values
  static val::BaseValue::PointerType get_scalar_boolean(const dip::Environment& env) {
    return std::make_unique<val::ArrayValue<bool>>(true);
  }
  static val::BaseValue::PointerType get_scalar_integer(const dip::Environment& env) {
    return std::make_unique<val::ArrayValue<int32_t>>(2);
  }
  static val::BaseValue::PointerType get_scalar_double(const dip::Environment& env) {
    return std::make_unique<val::ArrayValue<double>>(2.34e5);
  }
  static val::BaseValue::PointerType get_scalar_string(const dip::Environment& env) {
    return std::make_unique<val::ArrayValue<std::string>>("string");
  }

  // array values
  static val::BaseValue::PointerType get_array_boolean(const dip::Environment& env) {
    return val::create_array_value<bool>({false, true, false});
  }
  static val::BaseValue::PointerType get_array_integer(const dip::Environment& env) {
    return val::create_array_value<int>({2, 3, 4, 5}, {2, 2});
  }
  static val::BaseValue::PointerType get_array_double(const dip::Environment& env) {
    return val::create_array_value<double>({2.34e5, 3.45e6, 4.56e7});
  }
  static val::BaseValue::PointerType get_array_string(const dip::Environment& env) {
    return val::create_array_value<std::string>({"foo", "bar", "baz"});
  }

  // node lists
  static dip::BaseNode::NodeListType get_scalar_nodes(const dip::Environment& env) {
    return {
        dip::create_scalar_node<bool>("scalar_bool", false),
        dip::create_scalar_node<int>("scalar_int", 1),
        dip::create_scalar_node<double>("scalar_double", 2.34e5),
        dip::create_scalar_node<std::string>("scalar_str", "baz_value"),
    };
  }

  static dip::BaseNode::NodeListType get_array_nodes(const dip::Environment& env) {
    return {
        dip::create_array_node<bool>("array_bool", {false, true, false}),
        dip::create_array_node<int>("array_int", {1, 2, 3, 4}, {2, 2}),
        dip::create_array_node<double>("array_double", {2.34e5, 3.45e6, 4.56e7}),
        dip::create_array_node<std::string>("array_str", {"foo", "bar", "baz"}),
    };
  }
};

#endif // GTEST_FIXTURES_H
