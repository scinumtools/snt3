#include "pch_tests.h"

using namespace snt;

TEST(Math, Add) {

  std::vector<double> arr1 = {12.3, 2.3, 1.0345};
  std::vector<double> arr2 = {34e2, 0.004, 3};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr1);
  val::BaseValue::PointerType val2 = std::make_unique<val::ArrayValue<double>>(arr2);
  val::BaseValue::PointerType val3 = std::make_unique<val::ArrayValue<double>>(2);
  val::BaseValue::PointerType val4;

  // binary
  val4 = val1->math_add(val2.get()); // array + array
  EXPECT_EQ(val4->to_string(), "[3.412e3, 2.304, 4.034]");

  val4 = val3->math_add(val3.get()); // scalar + scalar
  EXPECT_EQ(val4->to_string(), "4");

  val4 = val1->math_add(val3.get()); // array + scalar
  EXPECT_EQ(val4->to_string(), "[14.3, 4.3, 3.034]");

  val4 = val3->math_add(val1.get()); // scalar + array
  EXPECT_EQ(val4->to_string(), "[14.3, 4.3, 3.034]");

  // binary assignment
  val1->math_add_equal(val2.get()); // array + array
  EXPECT_EQ(val1->to_string(), "[3.412e3, 2.304, 4.034]");

  val3->math_add_equal(val3.get()); // scalar + scalar
  EXPECT_EQ(val3->to_string(), "4");

  val1->math_add_equal(val3.get()); // array + scalar
  EXPECT_EQ(val1->to_string(), "[3.416e3, 6.304, 8.034]");

  val3->math_add_equal(val1.get()); // scalar + array
  EXPECT_EQ(val3->to_string(), "[3.42e3, 10.3, 12.03]");
}

TEST(Math, Sub) {

  std::vector<double> arr1 = {12.3, 2.3, 1.0345};
  std::vector<double> arr2 = {34e2, 0.004, 3};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr1);
  val::BaseValue::PointerType val2 = std::make_unique<val::ArrayValue<double>>(arr2);
  val::BaseValue::PointerType val4;

  val4 = val1->math_sub(val2.get());
  EXPECT_EQ(val4->to_string(), "[-3.388e3, 2.296, -1.966]");

  val1->math_sub_equal(val2.get());
  EXPECT_EQ(val1->to_string(), "[-3.388e3, 2.296, -1.966]");
}

TEST(Math, Neg) {

  std::vector<double> arr1 = {12.3, 2.3, 1.0345};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr1);
  val::BaseValue::PointerType val2;

  val2 = val1->math_neg();
  EXPECT_EQ(val2->to_string(), "[-12.3, -2.3, -1.034]");
}

TEST(Math, Mul) {

  std::vector<double> arr1 = {12.3, 2.3, 1.0345};
  std::vector<double> arr2 = {34e2, 0.004, 3};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr1);
  val::BaseValue::PointerType val2 = std::make_unique<val::ArrayValue<double>>(arr2);
  val::BaseValue::PointerType val4;

  val4 = val1->math_mul(val2.get());
  EXPECT_EQ(val4->to_string(), "[4.182e4, 9.2e-3, 3.103]");

  val1->math_mul_equal(val2.get());
  EXPECT_EQ(val1->to_string(), "[4.182e4, 9.2e-3, 3.103]");
}

TEST(Math, Div) {

  std::vector<double> arr1 = {12.3, 2.3, 1.0345};
  std::vector<double> arr2 = {34e2, 0.004, 3};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr1);
  val::BaseValue::PointerType val2 = std::make_unique<val::ArrayValue<double>>(arr2);
  val::BaseValue::PointerType val4;

  val4 = val1->math_div(val2.get());
  EXPECT_EQ(val4->to_string(), "[3.618e-3, 575, 0.3448]");

  val1->math_div_equal(val2.get());
  EXPECT_EQ(val1->to_string(), "[3.618e-3, 575, 0.3448]");
}

TEST(Math, Inv) {

  std::vector<double> arr1 = {12.3, 2.3, 1.0345};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr1);
  val::BaseValue::PointerType val2;

  val2 = val1->math_inv();
  EXPECT_EQ(val2->to_string(), "[0.0813, 0.4348, 0.9667]");
}

TEST(Math, Log) {

  std::vector<double> arr = {12e3, 2e4, 1.0345};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr);
  val::BaseValue::PointerType val2;

  val2 = val1->math_log();
  EXPECT_EQ(val2->to_string(), "[9.393, 9.903, 0.03392]");
}

TEST(Math, Log10) {

  std::vector<double> arr = {12.3, 2.3, 1.0345};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr);
  val::BaseValue::PointerType val2;

  val2 = val1->math_log10();
  EXPECT_EQ(val2->to_string(), "[1.09, 0.3617, 0.01473]");
}

TEST(Math, Sqrt) {

  std::vector<double> arr = {12e3, 2e4, 1.0345};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr);
  val::BaseValue::PointerType val2;

  val2 = val1->math_sqrt();
  EXPECT_EQ(val2->to_string(), "[109.5, 141.4, 1.017]");
}

TEST(Math, Cbrt) {

  std::vector<double> arr = {12.3, 2.3, 1.0345};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr);
  val::BaseValue::PointerType val2;

  val2 = val1->math_cbrt();
  EXPECT_EQ(val2->to_string(), "[2.308, 1.32, 1.011]");
}

TEST(Math, Exp) {

  std::vector<double> arr = {12.3, 2.3, 1.0345};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr);
  val::BaseValue::PointerType val2;

  val2 = val1->math_exp();
  EXPECT_EQ(val2->to_string(), "[2.197e5, 9.974, 2.814]");
}

TEST(Math, Floor) {

  std::vector<double> arr = {12.3, 2.3, 1.0345};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr);
  val::BaseValue::PointerType val2;

  val2 = val1->math_floor();
  EXPECT_EQ(val2->to_string(), "[12, 2, 1]");
}

TEST(Math, Ceil) {

  std::vector<double> arr = {12.3, 2.3, 1.0345};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<double>>(arr);
  val::BaseValue::PointerType val2;

  val2 = val1->math_ceil();
  EXPECT_EQ(val2->to_string(), "[13, 3, 2]");
}

TEST(Math, IntRealConversion) {

  std::vector<int> arr1 = {2, 3, 4};
  std::vector<double> arr2 = {3.4, 4.5, 5.6};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<int>>(arr1);
  val::BaseValue::PointerType val2 = std::make_unique<val::ArrayValue<double>>(arr2);
  val::BaseValue::PointerType val3;

  // addition
  val3 = val1->math_add(val2.get()); // int + real = real
  EXPECT_EQ(val3->to_string(), "[5.4, 7.5, 9.6]");
  // WARNING: the same is not possible for +=, -=, *= and \=, becaue the type of self is not mutable

  // substitution
  val3 = val1->math_sub(val2.get()); // int - real = real
  EXPECT_EQ(val3->to_string(), "[-1.4, -1.5, -1.6]");

  // multiplication
  val3 = val1->math_mul(val2.get()); // int * real = real
  EXPECT_EQ(val3->to_string(), "[6.8, 13.5, 22.4]");

  // division
  val3 = val1->math_div(val2.get()); // int / real = real
  EXPECT_EQ(val3->to_string(), "[0.5882, 0.6667, 0.7143]");
}
