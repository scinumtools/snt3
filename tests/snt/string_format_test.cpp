#include "snt/settings.h"
#include "snt/string_format.h"

#include <gtest/gtest.h>
#include <string>

TEST(StringFormat, Integers) {

  EXPECT_EQ(snt::number_to_string(int(123)), "123");
  EXPECT_EQ(snt::number_to_string(int(4567890)), "4567890");
  EXPECT_EQ(snt::number_to_string(int(-123)), "-123"); // negative values

  EXPECT_EQ(snt::number_to_string(int(123), int(2)), "123(2)");
  EXPECT_EQ(snt::number_to_string(int(4567890), int(234)), "456789(23)e1");
  EXPECT_EQ(snt::number_to_string(int(1234567890), int(12345)), "1234567(12)e3");
  EXPECT_EQ(snt::number_to_string(int(-123), int(2)), "-123(2)"); // negative values

  snt::StringFormatType prec;
  prec.errorPrecision = 5;
  EXPECT_EQ(snt::number_to_string(int(1234567890), int(12345), prec), "1234567890(12345)"); // extened precision
  EXPECT_EQ(snt::number_to_string(int(123), int(2), prec), "123(2)");                       // higher precision
}

TEST(StringFormat, Floats) {

  EXPECT_EQ(snt::number_to_string(double(1.23456789e-23)), "1.235e-23");
  EXPECT_EQ(snt::number_to_string(double(0.00000123456789)), "1.235e-6");
  EXPECT_EQ(snt::number_to_string(double(0.00123456789)), "1.235e-3");
  EXPECT_EQ(snt::number_to_string(double(0.0123456789)), "0.01235");
  EXPECT_EQ(snt::number_to_string(double(1.23456789)), "1.235");
  EXPECT_EQ(snt::number_to_string(double(123.456789)), "123.5");
  EXPECT_EQ(snt::number_to_string(double(1234.56789)), "1.235e3");
  EXPECT_EQ(snt::number_to_string(double(1234567.89)), "1.235e6");
  EXPECT_EQ(snt::number_to_string(double(1.23456789e23)), "1.235e23");

  EXPECT_EQ(snt::number_to_string(double(0.00123456789), double(0.000012)), "1.235(12)e-3");
  EXPECT_EQ(snt::number_to_string(double(12.67), double(0.02)), "1.2670(20)e1");
  EXPECT_EQ(snt::number_to_string(double(12345.67), double(2.23)), "1.23457(22)e4");

  snt::StringFormatType prec;
  prec.errorPrecision = 3;
  EXPECT_EQ(snt::number_to_string(double(12.3456), double(0.123), prec), "1.2346(123)e1"); // higher precision
}

TEST(StringFormat, BooleanArrays) {

  std::vector<bool> value = {true, false, true, false, true, false};
  std::vector<size_t> shape = {2, 3};
  EXPECT_EQ(snt::array_to_string(value, shape), "[[true, false, true], [false, true, false]]");
}

TEST(StringFormat, CharArrays) {

  std::vector<char> value = {'a', 'b', 'c', 'd', 'e', 'f'};
  std::vector<size_t> shape = {2, 3};
  EXPECT_EQ(snt::array_to_string(value, shape), "[[a, b, c], [d, e, f]]");
}

TEST(StringFormat, NumberArrays) {

  std::vector<double> value = {1, 2, 3, 4, 5, 6};
  std::vector<size_t> shape = {2, 3};
  EXPECT_EQ(snt::array_to_string(value, shape), "[[1, 2, 3], [4, 5, 6]]");

  value = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  shape = {2, 2, 3};
  EXPECT_EQ(snt::array_to_string(value, shape), "[[[1, 2, 3], [4, 5, 6]], [[7, 8, 9], [10, 11, 12]]]");
}

TEST(StringFormat, StringArrays) {

  std::vector<std::string> value = {"a", "b", "c", "d", "e", "f"};
  std::vector<size_t> shape = {2, 3};
  EXPECT_EQ(snt::array_to_string(value, shape), "[['a', 'b', 'c'], ['d', 'e', 'f']]");
}

TEST(StringFormat, ToString) {

  snt::StringFormatType format;
  std::vector<std::string> value = {"foo"};
  std::vector<size_t> shape = {1};

  // test string settings
  EXPECT_EQ(snt::array_to_string(value, shape), "'foo'");
  format.stringQuotes = false;
  EXPECT_EQ(snt::array_to_string(value, shape, format), "foo");

  // TODO: implement more format tests
}
