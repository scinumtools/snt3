#include <gtest/gtest.h>

#include "puq/magnitude.h"
#include "puq/nostd/nostd.h"

// test from https://www.quora.com/How-does-one-calculate-uncertainty-in-an-exponent
// and checked using http://www.julianibus.de/ online calculator

#ifdef MAGNITUDE_ERRORS

TEST(NoSTD, UnitValueMath) {

  puq::UnitValue uv1, uv2, uv3;

  puq::Magnitude m1(4.36, 0.16);
  puq::Magnitude m2(2.35, 0.04);
  uv1 = puq::UnitValue(m1, "m");
  uv2 = puq::UnitValue(m2, "m");

  // exponential function
  uv3 = puq::nostd::exp(uv2);
  EXPECT_EQ(uv3.to_string(), "1.049(42)e+01*m"); // mag 10.485569724727576 err 0.41942280901707824
}

TEST(NoSTD, MagnitudeMath) {

  puq::Magnitude m1, m2, m3;

  m1 = puq::Magnitude(4.36, 0.16);
  m2 = puq::Magnitude(2.35, 0.04);

  // exponential function
  m3 = puq::nostd::exp(m2);
  EXPECT_EQ(m3.to_string(), "1.049(42)e+01"); // mag 10.485569724727576 err 0.41942280901707824

  // power function with an exact exponent
  m3 = puq::nostd::pow(m1, 2.35);
  EXPECT_EQ(m3.to_string(), "3.18(27)e+01"); // mag 31.826820135086383 err 2.74469829832924

  // power function
  m3 = puq::nostd::pow(m1, m2);
  EXPECT_EQ(m3.to_string(), "3.18(33)e+01"); // mag 31.826820135086383 err 3.323756901862083

  // natural logarithm
  m3 = puq::nostd::log(m1);
  EXPECT_EQ(m3.to_string(), "1.472(37)"); // mag 1.472472057360943 err 0.03669724719657097

  // decadic logarithm
  m3 = puq::nostd::log10(m1);
  EXPECT_EQ(m3.to_string(), "6.39(16)e-01"); // mag 0.6394864892685861 err 0.01593741192351672

  // square root
  m3 = puq::nostd::sqrt(m1);
  EXPECT_EQ(m3.to_string(), "2.088(38)"); // mag 2.08806130178211 err 0.03831305122048434

  // cubic root
  m3 = puq::nostd::cbrt(m1);
  EXPECT_EQ(m3.to_string(), "1.634(20)"); // mag 1.633661834060757 err 0.019983631105446875

  // absolute value
  m3 = puq::nostd::abs(-m1);
  EXPECT_EQ(m3.to_string(), "4.36(16)");

  // maximum value
  m3 = puq::nostd::max(m1, m2);
  EXPECT_EQ(m3.to_string(), "4.36(16)");
}

#endif
