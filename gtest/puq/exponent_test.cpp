#include "pch_tests.h"

#ifdef EXPONENT_FRACTIONS

using namespace snt;

TEST(Exponent, Initialization) {

  puq::Exponent e = puq::Exponent();
  EXPECT_EQ(e.to_string(), "1"); // default unit exponent

  e = puq::Exponent(1);
  EXPECT_EQ(e.to_string(), "1"); // unit exponent

  e = puq::Exponent(0);
  EXPECT_EQ(e.to_string(), "0"); // zero exponent

  e = puq::Exponent(2);
  EXPECT_EQ(e.to_string(), "2"); // non-zero exponent

  e = puq::Exponent(3, 2);
  std::stringstream ss;
  ss << e; // cast exponent as a string into a stream
  EXPECT_EQ(ss.str(), "3:2");
}

TEST(Exponent, InitializationFractions) {

  puq::Exponent e = {1, 2};
  EXPECT_EQ(e.to_string(), "1:2"); // only denominator

  e = puq::Exponent(2, 1);
  EXPECT_EQ(e.to_string(), "2"); // only numerator

  e = puq::Exponent(2, -1);
  EXPECT_EQ(e.to_string(), "-2"); // negative denominator

  e = puq::Exponent(-2, 1);
  EXPECT_EQ(e.to_string(), "-2"); // negative numerator

  e = puq::Exponent(-2, -1);
  EXPECT_EQ(e.to_string(), "2"); // both negative

  e = puq::Exponent(0, 3);
  EXPECT_EQ(e.to_string(), "0"); // zero exponent

  e = puq::Exponent(30, 60);
  EXPECT_EQ(e.to_string(), "1:2"); // reduced form

  e = puq::Exponent(2 * 3 * 5, 3 * 5);
  EXPECT_EQ(e.to_string(), "2"); // reduced form
}

TEST(Exponent, ArithmeticsComparison) {

  puq::Exponent a(2);
  puq::Exponent b(1);

  EXPECT_EQ(a == b, false);
  EXPECT_EQ(a == a, true);

  EXPECT_EQ(a != b, true);
  EXPECT_EQ(a != a, false);
}

TEST(Exponent, ArithmeticsIntegers) {

  puq::Exponent a(2);
  puq::Exponent b(1);

  a += b;
  EXPECT_EQ(a.to_string(), "3");

  a -= b;
  EXPECT_EQ(a.to_string(), "2");
}

TEST(Exponent, ArithmeticsFractions) {

  puq::Exponent a(2, 3);
  puq::Exponent b(1, 6);

  a += b;
  EXPECT_EQ(a.to_string(), "5:6");

  a -= b;
  EXPECT_EQ(a.to_string(), "2:3");
}

#endif // EXPONENT_FRACTIONS
