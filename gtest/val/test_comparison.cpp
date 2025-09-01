#include "pch_tests.h"

using namespace snt;

TEST(Comparison, Equal) { // ==

  std::vector<bool> arr1 = {true, false, false};
  std::vector<bool> arr2 = {false, false, false};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<bool>>(arr1);
  val::BaseValue::PointerType val2 = std::make_unique<val::ArrayValue<bool>>(arr2);
  val::BaseValue::PointerType val3;

  val3 = val1->compare_equal(val1.get());
  EXPECT_EQ(val3->to_string(), "[true, true, true]");

  val3 = val1->compare_equal(val2.get());
  EXPECT_EQ(val3->to_string(), "[false, true, true]");
}

TEST(Comparison, NotEqual) { // !=

  std::vector<bool> arr1 = {true, false, false};
  std::vector<bool> arr2 = {false, false, false};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<bool>>(arr1);
  val::BaseValue::PointerType val2 = std::make_unique<val::ArrayValue<bool>>(arr2);
  val::BaseValue::PointerType val3;

  val3 = val1->compare_not_equal(val1.get());
  EXPECT_EQ(val3->to_string(), "[false, false, false]");

  val3 = val1->compare_not_equal(val2.get());
  EXPECT_EQ(val3->to_string(), "[true, false, false]");
}

TEST(Comparison, Greater) { // >

  std::vector<int> arr1 = {1, 2, 3};
  std::vector<int> arr2 = {3, 1, 2};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<int>>(arr1);
  val::BaseValue::PointerType val2 = std::make_unique<val::ArrayValue<int>>(arr2);
  val::BaseValue::PointerType val3;

  val3 = val1->compare_greater(val1.get());
  EXPECT_EQ(val3->to_string(), "[false, false, false]");

  val3 = val1->compare_greater(val2.get());
  EXPECT_EQ(val3->to_string(), "[false, true, true]");
}

TEST(Comparison, Less) { // <

  std::vector<int> arr1 = {1, 2, 3};
  std::vector<int> arr2 = {3, 1, 2};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<int>>(arr1);
  val::BaseValue::PointerType val2 = std::make_unique<val::ArrayValue<int>>(arr2);
  val::BaseValue::PointerType val3;

  val3 = val1->compare_less(val1.get());
  EXPECT_EQ(val3->to_string(), "[false, false, false]");

  val3 = val1->compare_less(val2.get());
  EXPECT_EQ(val3->to_string(), "[true, false, false]");
}

TEST(Comparison, GreaterEqual) { // >=

  std::vector<int> arr1 = {1, 2, 3};
  std::vector<int> arr2 = {3, 1, 2};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<int>>(arr1);
  val::BaseValue::PointerType val2 = std::make_unique<val::ArrayValue<int>>(arr2);
  val::BaseValue::PointerType val3;

  val3 = val1->compare_greater_equal(val1.get());
  EXPECT_EQ(val3->to_string(), "[true, true, true]");

  val3 = val1->compare_greater_equal(val2.get());
  EXPECT_EQ(val3->to_string(), "[false, true, true]");
}

TEST(Comparison, LessEqual) { // <=

  std::vector<int> arr1 = {1, 2, 3};
  std::vector<int> arr2 = {3, 1, 2};
  val::BaseValue::PointerType val1 = std::make_unique<val::ArrayValue<int>>(arr1);
  val::BaseValue::PointerType val2 = std::make_unique<val::ArrayValue<int>>(arr2);
  val::BaseValue::PointerType val3;

  val3 = val1->compare_less_equal(val1.get());
  EXPECT_EQ(val3->to_string(), "[true, true, true]");

  val3 = val1->compare_less_equal(val2.get());
  EXPECT_EQ(val3->to_string(), "[true, false, false]");
}
