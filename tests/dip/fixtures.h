#ifndef GTEST_FIXTURES_H
#define GTEST_FIXTURES_H

#include "pch_tests.h"

#include <snt/dip/environment.h>
#include <snt/dip/nodes/node_boolean.h>
#include <snt/dip/nodes/node_float.h>
#include <snt/dip/nodes/node_integer.h>
#include <snt/dip/nodes/node_string.h>
#include <snt/val/value_base.h>

using namespace snt;

class FixtureFunctions : public ::testing::Test {
  protected:
    // scalar values
    static val::BaseValue::PointerType get_scalar_boolean(const dip::Environment& env) {
        return std::make_unique<val::ArrayValueBool>(true);
    }
    static val::BaseValue::PointerType get_scalar_integer(const dip::Environment& env) {
        return std::make_unique<val::ArrayValueInt32>(2);
    }
    static val::BaseValue::PointerType get_scalar_double(const dip::Environment& env) {
        return std::make_unique<val::ArrayValueFloat64>(2.34e5);
    }
    static val::BaseValue::PointerType get_scalar_string(const dip::Environment& env) {
        return std::make_unique<val::ArrayValueStr>("string");
    }

    // array values
    static val::BaseValue::PointerType get_array_boolean(const dip::Environment& env) {
        std::vector<bool> val = {false, true, false};
        return std::make_unique<val::ArrayValueBool>(val);
    }
    static val::BaseValue::PointerType get_array_integer(const dip::Environment& env) {
        std::vector<int> val = {2, 3, 4, 5};
        val::Array::ShapeType sh = {2, 2};
        return std::make_unique<val::ArrayValueInt32>(val, sh);
    }
    static val::BaseValue::PointerType get_array_double(const dip::Environment& env) {
        std::vector<double> val = {2.34e5, 3.45e6, 4.56e7};
        return std::make_unique<val::ArrayValueFloat64>(val);
    }
    static val::BaseValue::PointerType get_array_string(const dip::Environment& env) {
        std::vector<std::string> val = {"foo", "bar", "baz"};
        return std::make_unique<val::ArrayValueStr>(val);
    }

    // node lists
    static dip::ValueNode::ListType get_scalar_nodes(const dip::Environment& env) {
        std::unique_ptr<val::ArrayValueBool> val_bool = std::make_unique<val::ArrayValueBool>(false);
        std::unique_ptr<val::ArrayValueInt32> val_int = std::make_unique<val::ArrayValueInt32>(1);
        std::unique_ptr<val::ArrayValueFloat64> val_double = std::make_unique<val::ArrayValueFloat64>(2.34e5);
        std::unique_ptr<val::ArrayValueStr> val_string = std::make_unique<val::ArrayValueStr>("baz_value");
        return {
            std::make_shared<dip::BooleanNode>(dip::Path("scalar_bool"), std::move(val_bool)),
            std::make_shared<dip::IntegerNode>(dip::Path("scalar_int"), std::move(val_int)),
            std::make_shared<dip::FloatNode>(dip::Path("scalar_double"), std::move(val_double)),
            std::make_shared<dip::StringNode>(dip::Path("scalar_str"), std::move(val_string)),
        };
    }

    static dip::ValueNode::ListType get_array_nodes(const dip::Environment& env) {
        std::vector<bool> vec_bool = {false, true, false};
        std::vector<int> vec_int = {1, 2, 3, 4};
        std::vector<double> vec_double = {2.34e5, 3.45e6, 4.56e7};
        std::vector<std::string> vec_string = {"foo", "bar", "baz"};
        val::Array::ShapeType sh = {2, 2};
        std::unique_ptr<val::ArrayValueBool> val_bool = std::make_unique<val::ArrayValueBool>(vec_bool);
        std::unique_ptr<val::ArrayValueInt32> val_int = std::make_unique<val::ArrayValueInt32>(vec_int, sh);
        std::unique_ptr<val::ArrayValueFloat64> val_double = std::make_unique<val::ArrayValueFloat64>(vec_double);
        std::unique_ptr<val::ArrayValueStr> val_string = std::make_unique<val::ArrayValueStr>(vec_string);
        return {
            std::make_shared<dip::BooleanNode>(dip::Path("array_bool"), std::move(val_bool)),
            std::make_shared<dip::IntegerNode>(dip::Path("array_int"), std::move(val_int)),
            std::make_shared<dip::FloatNode>(dip::Path("array_double"), std::move(val_double)),
            std::make_shared<dip::StringNode>(dip::Path("array_str"), std::move(val_string)),
        };
    }
};

#endif // GTEST_FIXTURES_H
