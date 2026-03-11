#include "pch_tests.h"
#include <snt/puq/nostd/abs.h>
#include <snt/puq/nostd/cbrt.h>
#include <snt/puq/nostd/exp.h>
#include <snt/puq/nostd/log.h>
#include <snt/puq/nostd/log10.h>
#include <snt/puq/nostd/max.h>
#include <snt/puq/nostd/pow.h>
#include <snt/puq/nostd/sqrt.h>
#include <snt/puq/nostd/to_string.h>

using namespace snt;

// test from https://www.quora.com/How-does-one-calculate-uncertainty-in-an-exponent
// and checked using http://www.julianibus.de/ online calculator

TEST(NoSTD, MeasurementMathUnits) {

  puq::Measurement msr1, msr2, msr3;

  msr1 = puq::Measurement(4.36, 0.16, "m");
  msr2 = puq::Measurement(2.35, 0.04, "m");

  // exponential function
  msr3 = nostd::exp(msr2);
  EXPECT_EQ(msr3.to_string(), "1.049(42)e1*m"); // mag 10.485569724727576 err 0.41942280901707824
}

TEST(NoSTD, MeasurementMath) {

  puq::Measurement msr1, msr2, msr3;

  msr1 = puq::Measurement(4.36, 0.16);
  msr2 = puq::Measurement(2.35, 0.04);

  // exponential function
  msr3 = nostd::exp(msr2);
  EXPECT_EQ(msr3.to_string(), "1.049(42)e1"); // est 10.485569724727576 unc 0.41942280901707824

  // power function with an exact exponent
  msr3 = nostd::pow(msr1, 2.35);
  EXPECT_EQ(msr3.to_string(), "3.18(27)e1"); // est 31.826820135086383 unc 2.74469829832924

  // power function
  msr3 = nostd::pow(msr1, msr2);
  EXPECT_EQ(msr3.to_string(), "3.18(33)e1"); // est 31.826820135086383 unc 3.323756901862083

  // natural logarithm
  msr3 = nostd::log(msr1);
  EXPECT_EQ(msr3.to_string(), "1.472(37)"); // est 1.472472057360943 unc 0.03669724719657097

  // decadic logarithm
  msr3 = nostd::log10(msr1);
  EXPECT_EQ(msr3.to_string(), "6.39(16)e-1"); // est 0.6394864892685861 unc 0.01593741192351672

  // square root
  msr3 = nostd::sqrt(msr1);
  EXPECT_EQ(msr3.to_string(), "2.088(38)"); // est 2.08806130178211 unc 0.03831305122048434

  // cubic root
  msr3 = nostd::cbrt(msr1);
  EXPECT_EQ(msr3.to_string(), "1.634(20)"); // est 1.633661834060757 unc 0.019983631105446875

  // absolute value
  msr3 = nostd::abs(-msr1);
  EXPECT_EQ(msr3.to_string(), "4.36(16)");

  // maximum value
  msr3 = nostd::max(msr1, msr2);
  EXPECT_EQ(msr3.to_string(), "4.36(16)");
}
