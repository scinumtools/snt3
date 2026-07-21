#include "pch_tests.h"

#include <snt/puq/quantity.h>

using namespace snt;

TEST(QuickExample, ConvertQuantity) {

    puq::Quantity length("3.048*m");
    length = length.convert("US_ft");
    EXPECT_EQ(length.to_string(), "10*ft");
}
