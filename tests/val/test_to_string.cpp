#include "pch_tests.h"

#include <snt/val/to_string.h>
#include <snt/val/values.h>

using namespace snt;

TEST(ToString, BaseValue) {

    std::vector<float> arr = {1.234567, 2.34567};
    val::BaseValue::PointerType val = std::make_unique<val::ArrayValueFloat32>(arr);

    EXPECT_EQ(val::to_string(std::move(val), 5), "[1.2346, 2.3457]");
}

TEST(ToString, ArrayShape) {

    val::Array::ShapeType sh{2, 3};

    EXPECT_EQ(val::to_string(sh), "[2,3]");
}
