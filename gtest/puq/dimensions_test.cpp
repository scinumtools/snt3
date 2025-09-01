#include "pch_tests.h"

using namespace snt;

TEST(Dimensions, Initialization) {

  puq::Dimensions d;
  EXPECT_EQ(d.to_string(), "1");

  d = puq::Dimensions(2.34);
  EXPECT_EQ(d.to_string(), "2.34");

  d = puq::Dimensions(2.34, {0, 1, 2, 3, 4, 5, 6, 7});
  EXPECT_EQ(d.to_string(), "2.34*g*s2*K3*A4*cd5*mol6*rad7");

  d = puq::Dimensions(2, {0, 1, 2, 0, 0, 0, 0, 0});
  std::stringstream ss;
  ss << d; // cast diemensions as a string into a stream
  EXPECT_EQ(ss.str(), "2*g*s2");
}

#ifdef MAGNITUDE_ERRORS

TEST(Dimensions, InitializationErrors) {

  puq::Dimensions d(2.34, 0.23);
  EXPECT_EQ(d.to_string(), "2.34(23)");

  d = puq::Dimensions(2.34, 0.23, {0, 1, 2, 3, 4, 5, 6, 7});
  EXPECT_EQ(d.to_string(), "2.34(23)*g*s2*K3*A4*cd5*mol6*rad7");
}

#endif

#ifdef EXPONENT_FRACTIONS

TEST(Dimensions, InitializationFractions) {

  puq::Dimensions d(2.34, {1, {1, -2}, 0, 0, 0, 0, 0, 0});
  EXPECT_EQ(d.to_string(), "2.34*m*g-1:2");

  d = puq::Dimensions(2.34, puq::BaseDimensions({puq::Exponent(3, 2), {1, -2}, 0, 0, 0, 0, 0, 0}));
  EXPECT_EQ(d.to_string(), "2.34*m3:2*g-1:2");
}

#endif

TEST(Dimensions, Comparison) {

  puq::Dimensions d1(2.34, {0, 1, 2, 3, 4, 5, 6, 7});
  puq::Dimensions d2(2.34, {0, 0, 1, 2, 3, 4, 5, 6});

  EXPECT_EQ(d1 == d2, false);
  EXPECT_EQ(d1 == d1, true);

  EXPECT_EQ(d1 != d2, true);
  EXPECT_EQ(d1 != d1, false);
}
