#include "pch_tests.h"

using namespace snt;

TEST(Slicing, Basic) {

  std::vector<int> arr = {1, 2, 3, 4, 5, 6};
  val::Array::ShapeType shape({2, 3});
  val::BaseValue::PointerType val = std::make_unique<val::ArrayValue<int>>(arr, shape);
  EXPECT_EQ(val->to_string(), "[[1, 2, 3], [4, 5, 6]]");
  val::BaseValue::PointerType bval;

  val::Array::RangeType slice({
      val::Array::RangeStruct({0, 0}),
      val::Array::RangeStruct({0, 1}),
  });
  bval = val->slice(slice);
  EXPECT_EQ(bval->to_string(), "[1, 2]");
}
