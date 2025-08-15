#include <gtest/gtest.h>

#include "val/values.h"

TEST(Math, Add) {

  std::vector<double> arr1 = {12.3, 2.3, 1.0345};
  std::vector<double> arr2 = {34e2, 0.004, 3};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr1);
  val::BaseValue::PointerType val2 = std::make_unique<val::ArrayValue<double>>(arr2);
  val::BaseValue::PointerType val3 = std::make_unique<val::ArrayValue<double>>(2);
  val::BaseValue::PointerType val4;

  // binary
  val4 = val1->math_add(val2.get()); // array + array
  EXPECT_EQ(val4->to_string(), "[3412.3, 2.3040, 4.0345]");

  val4 = val3->math_add(val3.get()); // scalar + scalar
  EXPECT_EQ(val4->to_string(), "4.0000");

  val4 = val1->math_add(val3.get()); // array + scalar
  EXPECT_EQ(val4->to_string(), "[14.300, 4.3000, 3.0345]");

  val4 = val3->math_add(val1.get()); // scalar + array
  EXPECT_EQ(val4->to_string(), "[14.300, 4.3000, 3.0345]");

  // binary assignment
  val1->math_add_equal(val2.get()); // array + array
  EXPECT_EQ(val1->to_string(), "[3412.3, 2.3040, 4.0345]");

  val3->math_add_equal(val3.get()); // scalar + scalar
  EXPECT_EQ(val3->to_string(), "4.0000");

  val1->math_add_equal(val3.get()); // array + scalar
  EXPECT_EQ(val1->to_string(), "[3416.3, 6.3040, 8.0345]");

  val3->math_add_equal(val1.get()); // scalar + array
  EXPECT_EQ(val3->to_string(), "[3420.3, 10.304, 12.034]");
}

TEST(Math, Sub) {

  std::vector<double> arr1 = {12.3, 2.3, 1.0345};
  std::vector<double> arr2 = {34e2, 0.004, 3};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr1);
  val::BaseValue::PointerType val2 = std::make_unique<val::ArrayValue<double>>(arr2);
  val::BaseValue::PointerType val4;

  val4 = val1->math_sub(val2.get());
  EXPECT_EQ(val4->to_string(), "[-3387.7, 2.2960, -1.9655]");

  val1->math_sub_equal(val2.get());
  EXPECT_EQ(val1->to_string(), "[-3387.7, 2.2960, -1.9655]");
}

TEST(Math, Mul) {

  std::vector<double> arr1 = {12.3, 2.3, 1.0345};
  std::vector<double> arr2 = {34e2, 0.004, 3};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr1);
  val::BaseValue::PointerType val2 = std::make_unique<val::ArrayValue<double>>(arr2);
  val::BaseValue::PointerType val4;

  val4 = val1->math_mul(val2.get());
  EXPECT_EQ(val4->to_string(), "[4.1820e+04, 0.009200, 3.1035]");

  val1->math_mul_equal(val2.get());
  EXPECT_EQ(val1->to_string(), "[4.1820e+04, 0.009200, 3.1035]");
}

TEST(Math, Div) {

  std::vector<double> arr1 = {12.3, 2.3, 1.0345};
  std::vector<double> arr2 = {34e2, 0.004, 3};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr1);
  val::BaseValue::PointerType val2 = std::make_unique<val::ArrayValue<double>>(arr2);
  val::BaseValue::PointerType val4;

  val4 = val1->math_div(val2.get());
  EXPECT_EQ(val4->to_string(), "[0.003618, 575.00, 0.3448]");

  val1->math_div_equal(val2.get());
  EXPECT_EQ(val1->to_string(), "[0.003618, 575.00, 0.3448]");
}

TEST(Math, Log) {

  std::vector<double> arr = {12e3, 2e4, 1.0345};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr);
  val::BaseValue::PointerType val2;

  val2 = val1->math_log();
  EXPECT_EQ(val2->to_string(), "[9.3927, 9.9035, 0.03392]");
}

TEST(Math, Log10) {

  std::vector<double> arr = {12.3, 2.3, 1.0345};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr);
  val::BaseValue::PointerType val2;

  val2 = val1->math_log10();
  EXPECT_EQ(val2->to_string(), "[1.0899, 0.3617, 0.01473]");
}

TEST(Math, Sqrt) {

  std::vector<double> arr = {12e3, 2e4, 1.0345};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr);
  val::BaseValue::PointerType val2;

  val2 = val1->math_sqrt();
  EXPECT_EQ(val2->to_string(), "[109.54, 141.42, 1.0171]");
}

TEST(Math, Cbrt) {

  std::vector<double> arr = {12.3, 2.3, 1.0345};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr);
  val::BaseValue::PointerType val2;

  val2 = val1->math_cbrt();
  EXPECT_EQ(val2->to_string(), "[2.3084, 1.3200, 1.0114]");
}

TEST(Math, Exp) {

  std::vector<double> arr = {12.3, 2.3, 1.0345};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr);
  val::BaseValue::PointerType val2;

  val2 = val1->math_exp();
  EXPECT_EQ(val2->to_string(), "[2.1970e+05, 9.9742, 2.8137]");
}

TEST(Math, Floor) {

  std::vector<double> arr = {12.3, 2.3, 1.0345};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr);
  val::BaseValue::PointerType val2;

  val2 = val1->math_floor();
  EXPECT_EQ(val2->to_string(), "[12.000, 2.0000, 1.0000]");
}

TEST(Math, Ceil) {

  std::vector<double> arr = {12.3, 2.3, 1.0345};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr);
  val::BaseValue::PointerType val2;

  val2 = val1->math_ceil();
  EXPECT_EQ(val2->to_string(), "[13.000, 3.0000, 2.0000]");
}
