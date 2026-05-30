#include "pch_tests.h"

#include <snt/val/values.h>

using namespace snt;

TEST(Casting, Boolean) {

    std::vector<bool> arr = {true, false, false, true, true};
    val::BaseValue::PointerType val = std::make_unique<val::ArrayValueBool>(arr);
    val::BaseValue::PointerType bval;

    bval = val->cast_as(core::DataType::Boolean);
    EXPECT_EQ(bval->to_string(), "[true, false, false, true, true]");

    bval = val->cast_as(core::DataType::Integer32);
    EXPECT_EQ(bval->to_string(), "[1, 0, 0, 1, 1]");

    bval = val->cast_as(core::DataType::Float64);
    EXPECT_EQ(bval->to_string(), "[1, 0, 0, 1, 1]");

    bval = val->cast_as(core::DataType::String);
    EXPECT_EQ(bval->to_string(), "[\"true\", \"false\", \"false\", \"true\", \"true\"]");

    val = std::make_unique<val::ArrayValueBool>(1);
    bval = val->cast_as(core::DataType::Character);
    EXPECT_EQ(bval->to_string(), "\x1");
}

TEST(Casting, Character) {

    std::vector<char> arr = {65, 66, 67, 68, 69};
    val::BaseValue::PointerType val = std::make_unique<val::ArrayValueChar>(arr);
    val::BaseValue::PointerType bval;

    bval = val->cast_as(core::DataType::Boolean);
    EXPECT_EQ(bval->to_string(), "[true, true, true, true, true]");

    bval = val->cast_as(core::DataType::Integer32);
    EXPECT_EQ(bval->to_string(), "[65, 66, 67, 68, 69]");

    bval = val->cast_as(core::DataType::Float64);
    EXPECT_EQ(bval->to_string(), "[65, 66, 67, 68, 69]");

    bval = val->cast_as(core::DataType::String);
    EXPECT_EQ(bval->to_string(), "[\"65\", \"66\", \"67\", \"68\", \"69\"]");

    bval = val->cast_as(core::DataType::Character);
    EXPECT_EQ(bval->to_string(), "[A, B, C, D, E]");
}

TEST(Casting, Integer) {

    std::vector<int> arr = {0, 2, 33, 45, 100023};
    val::BaseValue::PointerType val = std::make_unique<val::ArrayValueInt32>(arr);
    val::BaseValue::PointerType bval;

    bval = val->cast_as(core::DataType::Boolean);
    EXPECT_EQ(bval->to_string(), "[false, true, true, true, true]");

    bval = val->cast_as(core::DataType::Integer32);
    EXPECT_EQ(bval->to_string(), "[0, 2, 33, 45, 100023]");

    bval = val->cast_as(core::DataType::Float64);
    EXPECT_EQ(bval->to_string(), "[0, 2, 33, 45, 1e5]");

    bval = val->cast_as(core::DataType::String);
    EXPECT_EQ(bval->to_string(), "[\"0\", \"2\", \"33\", \"45\", \"100023\"]");

    val = std::make_unique<val::ArrayValueInt32>(66);
    bval = val->cast_as(core::DataType::Character);
    EXPECT_EQ(bval->to_string(), "B");
}

TEST(Casting, Float) {

    std::vector<double> arr = {0, 2, 33.3, 45e2, -100023e3};
    val::BaseValue::PointerType val = std::make_unique<val::ArrayValueFloat64>(arr);
    val::BaseValue::PointerType bval;

    bval = val->cast_as(core::DataType::Boolean);
    EXPECT_EQ(bval->to_string(), "[false, true, true, true, true]");

    bval = val->cast_as(core::DataType::Integer32);
    EXPECT_EQ(bval->to_string(), "[0, 2, 33, 4500, -100023000]");

    bval = val->cast_as(core::DataType::Float64);
    EXPECT_EQ(bval->to_string(), "[0, 2, 33.3, 4.5e3, -1e8]");

    bval = val->cast_as(core::DataType::String);
    EXPECT_EQ(bval->to_string(), "[\"0.000000\", \"2.000000\", \"33.300000\", \"4500.000000\", \"-100023000.000000\"]");

    val = std::make_unique<val::ArrayValueFloat64>(66.);
    bval = val->cast_as(core::DataType::Character);
    EXPECT_EQ(bval->to_string(), "B");
}

TEST(Casting, String) {

    std::vector<std::string> arr;
    val::BaseValue::PointerType val;
    val::BaseValue::PointerType bval;

    arr = {"true", "false"};
    val = std::make_unique<val::ArrayValueStr>(arr);
    bval = val->cast_as(core::DataType::Boolean);
    EXPECT_EQ(bval->to_string(), "[true, false]");

    arr = {"1234", "-5678"};
    val = std::make_unique<val::ArrayValueStr>(arr);
    bval = val->cast_as(core::DataType::Integer32);
    EXPECT_EQ(bval->to_string(), "[1234, -5678]");

    arr = {"1", "1.2", "1.23e4"};
    val = std::make_unique<val::ArrayValueStr>(arr);
    bval = val->cast_as(core::DataType::Float64);
    EXPECT_EQ(bval->to_string(), "[1, 1.2, 1.23e4]");

    arr = {"a", "b", "c"};
    val = std::make_unique<val::ArrayValueStr>(arr);
    bval = val->cast_as(core::DataType::String);
    EXPECT_EQ(bval->to_string(), "[\"a\", \"b\", \"c\"]");

    val = std::make_unique<val::ArrayValueStr>("B");
    bval = val->cast_as(core::DataType::Character);
    EXPECT_EQ(bval->to_string(), "B");
}
