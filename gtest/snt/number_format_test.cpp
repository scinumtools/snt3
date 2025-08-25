#include "snt/settings.h"

#include <gtest/gtest.h>

TEST(NumberFormat, Integers) {

  EXPECT_EQ(snt::number_to_string(int(123)),    "123");
  EXPECT_EQ(snt::number_to_string(int(4567890)), "4567890");
}

TEST(NumberFormat, FloatingPoint) {

  EXPECT_EQ(snt::number_to_string(double(1.23456789e-23)),   "1.235e-23");
  EXPECT_EQ(snt::number_to_string(double(0.00000123456789)), "1.235e-6");
  EXPECT_EQ(snt::number_to_string(double(0.00123456789)),    "1.235e-3");
  EXPECT_EQ(snt::number_to_string(double(0.0123456789)),     "0.01235");
  EXPECT_EQ(snt::number_to_string(double(1.23456789)),       "1.235");
  EXPECT_EQ(snt::number_to_string(double(123.456789)),       "123.5");
  EXPECT_EQ(snt::number_to_string(double(1234.56789)),       "1.235e3");
  EXPECT_EQ(snt::number_to_string(double(1234567.89)),       "1.235e6");
  EXPECT_EQ(snt::number_to_string(double(1.23456789e23)),    "1.235e23");
}
