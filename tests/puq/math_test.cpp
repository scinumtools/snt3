#include "pch_tests.h"
#include <snt/puq/result.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/math.h>
#include <snt/puq/to_string.h>

using namespace snt;

// test from https://www.quora.com/How-does-one-calculate-uncertainty-in-an-exponent
// and checked using http://www.julianibus.de/ online calculator

TEST(Math, Power) {

  // Base Units
  puq::BaseUnits bu0;
  puq::BaseUnits bu1("kg*m2/s2");
  puq::Exponent exp(1,2);
  bu0 = puq::math::pow(bu1, exp);
  EXPECT_EQ(bu0.to_string(), "kg1:2*m*s-1");

  // Result
  puq::Result res0;
  puq::Result res1(4.36, 0.16);
  puq::Result res2(2.35, 0.04);
  res0 = puq::math::pow(res1, 2.35);         // with a float exponent
  EXPECT_EQ(res0.to_string(), "3.18(27)e1"); // est 31.826820135086383 unc 2.74469829832924
  res0 = puq::math::pow(res1, res2);         // with an result expoenent
  EXPECT_EQ(res0.to_string(), "3.18(33)e1"); // est 31.826820135086383 unc 3.323756901862083
}

TEST(Math, Exponent) {

  // Result
  puq::Result res0;
  puq::Result res1(2.35, 0.04);
  res0 = puq::math::exp(res1);
  EXPECT_EQ(res0.to_string(), "1.049(42)e1"); // est 10.485569724727576 unc 0.41942280901707824

  // Measurement
  puq::Measurement msr0;
  puq::Measurement msr1(7.23, 0.07);
  puq::Measurement msr2(2.35, 0.04, "km3");
  msr0 = puq::math::exp(msr1);
  EXPECT_EQ(msr0.to_string(), "1.380(97)e3");
  try {
    puq::math::exp(msr2);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_STREQ(e.what(), "Exponential function accepts only dimmensionless quatntities.");
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }

  // Quantity    TODO: implement tests with different systems
  puq::Quantity quant0;
  puq::Quantity quant1(2.35, 0.04);
  quant0 = puq::math::exp(quant1);
  EXPECT_EQ(quant0.to_string(), "1.049(42)e1");
}

TEST(Math, AbsoluteValue) {

  // Result
  puq::Result res0;
  puq::Result res1(-4.36, 0.16);
  res0 = puq::math::abs(res1);
  EXPECT_EQ(res0.to_string(), "4.36(16)");

  // Measurement
  puq::Measurement msr0;
  puq::Measurement msr1(-4.36, 0.16, "m2");
  msr0 = puq::math::abs(msr1);
  EXPECT_EQ(msr0.to_string(), "4.36(16)*m2");

  // Quantity    TODO: implement tests with different systems
  puq::Quantity quant0;
  puq::Quantity quant1(-4.36, 0.16, "m");
  quant0 = puq::math::abs(quant1);
  EXPECT_EQ(quant0.to_string(), "4.36(16)*m");
}

TEST(Math, LogarithmNatural) {

  // Result
  puq::Result res0;
  puq::Result res1(4.36, 0.16);
  res0 = puq::math::log(res1);
  EXPECT_EQ(res0.to_string(), "1.472(37)"); // est 1.472472057360943 unc 0.03669724719657097
}

TEST(Math, LogarithmDecadic) {

  // Result
  puq::Result res0;
  puq::Result res1(4.36, 0.16);
  res0 = puq::math::log10(res1);
  EXPECT_EQ(res0.to_string(), "6.39(16)e-1"); // est 0.6394864892685861 unc 0.01593741192351672
}
  
TEST(Math, Maximum) {
  
  // Result
  puq::Result res0;
  puq::Result res1(4.36, 0.16);
  puq::Result res2(2.35, 0.04);
  res0 = puq::math::max(res1, res2);
  EXPECT_EQ(res0.to_string(), "4.36(16)");
  
  // Measurement
  //puq::Measurement msr0;
  //puq::Measurement msr1(2.35, 0.04, "km")
  //puq::Measurement msr2(3.45, 0.3, "m")
  //msr0 = puq::math::max(msr1, msr2);
  //EXPECT_EQ(msr0.to_string(), "2.35(4)*km");
}

TEST(Math, CubicRoot) {

  // Result
  puq::Result res0;
  puq::Result res1(4.36, 0.16);
  res0 = puq::math::cbrt(res1);
  EXPECT_EQ(res0.to_string(), "1.634(20)");        // est 1.633661834060757 unc 0.019983631105446875
  
  // Measurement
  puq::Measurement msr0;
  puq::Measurement msr1(2.35, 0.04, "km3");
  msr0 = puq::math::cbrt(msr1);
  EXPECT_EQ(msr0.to_string(), "1.3295(75)*km");   // est 1.3295028952345866 unc 0.0075432788015916685  
}

TEST(Math, SquareRoot) {

  // Result
  puq::Result res0;
  puq::Result res1(4.36, 0.16);
  res0 = puq::math::sqrt(res1);
  EXPECT_EQ(res0.to_string(), "2.088(38)");       // est 2.08806130178211 unc 0.03831305122048434
  
  // Measurement
  puq::Measurement msr0;
  puq::Measurement msr1(4.36, 0.16, "m2");
  msr0 = puq::math::sqrt(msr1);
  EXPECT_EQ(msr0.to_string(), "2.088(38)*m");     // est 2.08806130178211 unc 0.03831305122048434
}
