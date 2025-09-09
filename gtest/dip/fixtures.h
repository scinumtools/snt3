#ifndef GTEST_FIXTURES_H
#define GTEST_FIXTURES_H

#include "pch_tests.h"

using namespace snt;

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
    std::vector<bool> val = {false, true, false};
    return std::make_unique<val::ArrayValue<bool>>(val);
  }
  static val::BaseValue::PointerType get_array_integer(const dip::Environment& env) {
    std::vector<int> val = {2, 3, 4, 5};
    val::Array::ShapeType sh = {2, 2};
    return std::make_unique<val::ArrayValue<int>>(val, sh);
  }
  static val::BaseValue::PointerType get_array_double(const dip::Environment& env) {
    std::vector<double> val = {2.34e5, 3.45e6, 4.56e7};
    return std::make_unique<val::ArrayValue<double>>(val);
  }
  static val::BaseValue::PointerType get_array_string(const dip::Environment& env) {
    std::vector<std::string> val = {"foo", "bar", "baz"};
    return std::make_unique<val::ArrayValue<std::string>>(val);
  }

  // node lists
  static dip::BaseNode::NodeListType get_scalar_nodes(const dip::Environment& env) {
    std::unique_ptr<val::ArrayValue<bool>> val_bool =
        std::make_unique<val::ArrayValue<bool>>(false);
    std::unique_ptr<val::ArrayValue<int>> val_int =
        std::make_unique<val::ArrayValue<int>>(1);
    std::unique_ptr<val::ArrayValue<double>> val_double =
        std::make_unique<val::ArrayValue<double>>(2.34e5);
    std::unique_ptr<val::ArrayValue<std::string>> val_string =
        std::make_unique<val::ArrayValue<std::string>>("baz_value");
    return {
        std::make_shared<dip::BooleanNode>("scalar_bool", std::move(val_bool)),
        std::make_shared<dip::IntegerNode>("scalar_int", std::move(val_int)),
        std::make_shared<dip::RealNode>("scalar_double", std::move(val_double)),
        std::make_shared<dip::StringNode>("scalar_str", std::move(val_string)),
    };
  }

  static dip::BaseNode::NodeListType get_array_nodes(const dip::Environment& env) {
    std::vector<bool> vec_bool = {false, true, false};
    std::vector<int> vec_int = {1, 2, 3, 4};
    std::vector<double> vec_double = {2.34e5, 3.45e6, 4.56e7};
    std::vector<std::string> vec_string = {"foo", "bar", "baz"};
    val::Array::ShapeType sh = {2, 2};
    std::unique_ptr<val::ArrayValue<bool>> val_bool =
        std::make_unique<val::ArrayValue<bool>>(vec_bool);
    std::unique_ptr<val::ArrayValue<int>> val_int =
        std::make_unique<val::ArrayValue<int>>(vec_int, sh);
    std::unique_ptr<val::ArrayValue<double>> val_double =
        std::make_unique<val::ArrayValue<double>>(vec_double);
    std::unique_ptr<val::ArrayValue<std::string>> val_string =
        std::make_unique<val::ArrayValue<std::string>>(vec_string);
    return {
        std::make_shared<dip::BooleanNode>("array_bool", std::move(val_bool)),
        std::make_shared<dip::IntegerNode>("array_int", std::move(val_int)),
        std::make_shared<dip::RealNode>("array_double", std::move(val_double)),
        std::make_shared<dip::StringNode>("array_str", std::move(val_string)),
    };
  }
};

#endif // GTEST_FIXTURES_H
