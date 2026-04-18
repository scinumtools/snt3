#include "pch_tests.h"
#include <snt/puq/result.h>
#include <snt/puq/value/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/math/abs.h>
#include <snt/puq/math/cbrt.h>
#include <snt/puq/math/exp.h>
#include <snt/puq/math/log.h>
#include <snt/puq/math/log10.h>
#include <snt/puq/math/max.h>
#include <snt/puq/math/pow.h>
#include <snt/puq/math/sqrt.h>
#include <snt/puq/to_string.h>

using namespace snt;

// test from https://www.quora.com/How-does-one-calculate-uncertainty-in-an-exponent
// and checked using http://www.julianibus.de/ online calculator

TEST(Math, Measurement) {

  puq::Measurement msr1, msr2, msr3;

  msr1 = puq::Measurement(4.36, 0.16);
  msr2 = puq::Measurement(2.35, 0.04);

  // exponential function
  msr3 = puq::math::exp(msr2);
  EXPECT_EQ(msr3.to_string(), "1.049(42)e1"); // est 10.485569724727576 unc 0.41942280901707824

  // power function with an exact exponent
  msr3 = puq::math::pow(msr1, 2.35);
  EXPECT_EQ(msr3.to_string(), "3.18(27)e1"); // est 31.826820135086383 unc 2.74469829832924

  // power function
  msr3 = puq::math::pow(msr1, msr2);
  EXPECT_EQ(msr3.to_string(), "3.18(33)e1"); // est 31.826820135086383 unc 3.323756901862083

  // natural logarithm
  msr3 = puq::math::log(msr1);
  EXPECT_EQ(msr3.to_string(), "1.472(37)"); // est 1.472472057360943 unc 0.03669724719657097

  // decadic logarithm
  msr3 = puq::math::log10(msr1);
  EXPECT_EQ(msr3.to_string(), "6.39(16)e-1"); // est 0.6394864892685861 unc 0.01593741192351672

  // square root
  msr3 = puq::math::sqrt(msr1);
  EXPECT_EQ(msr3.to_string(), "2.088(38)"); // est 2.08806130178211 unc 0.03831305122048434

  // cubic root
  msr3 = puq::math::cbrt(msr1);
  EXPECT_EQ(msr3.to_string(), "1.634(20)"); // est 1.633661834060757 unc 0.019983631105446875

  // absolute value
  msr3 = puq::math::abs(-msr1);
  EXPECT_EQ(msr3.to_string(), "4.36(16)");

  // maximum value
  msr3 = puq::math::max(msr1, msr2);
  EXPECT_EQ(msr3.to_string(), "4.36(16)");
}

TEST(Math, MeasurementUnits) {

  puq::Measurement msr1, msr2, msr3;

  msr1 = puq::Measurement(4.36, 0.16, "m");
  msr2 = puq::Measurement(2.35, 0.04, "km");

  // exponential function
  msr3 = puq::math::exp(msr2);
  EXPECT_EQ(msr3.to_string(), "1.049(42)e1*km"); // mag 10.485569724727576 err 0.41942280901707824

  // absolute value
  msr3 = puq::math::abs(-msr1);
  EXPECT_EQ(msr3.to_string(), "4.36(16)*m");
  
  // maximum value
  //msr3 = puq::math::max(msr1, msr2);
  //EXPECT_EQ(msr3.to_string(), "2.35(4)*km");
}

TEST(Math, Quantity) {

  puq::Quantity quant1, quant2, quant3;

  quant1 = puq::Quantity(4.36, 0.16, "m");
  quant2 = puq::Quantity(2.35, 0.04, "km");

  // exponential function
  quant3 = puq::math::exp(quant2);
  EXPECT_EQ(quant3.to_string(), "1.049(42)e1*km");
  
  // absolute value
  quant3 = puq::math::abs(-quant1);
  EXPECT_EQ(quant3.to_string(), "4.36(16)*m");

  // DEBUG: implement tests with different systems
}
