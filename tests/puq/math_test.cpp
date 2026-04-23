#include "pch_tests.h"

#include <snt/puq/math.h>
#include <snt/puq/measurement.h>
#include <snt/puq/quantity.h>
#include <snt/puq/result.h>
#include <snt/puq/to_string.h>

using namespace snt;

// test from https://www.quora.com/How-does-one-calculate-uncertainty-in-an-exponent
// and checked using http://www.julianibus.de/ online calculator

TEST(Math, Power) {

  {
    // Base Units
    puq::BaseUnits bu0;
    puq::BaseUnits bu1("kg*m2/s2");
    puq::Exponent exp(1, 2);
    bu0 = puq::math::pow(bu1, exp);
    EXPECT_EQ(bu0.to_string(), "kg1:2*m*s-1");
  }
  {
    // Result
    puq::Result res0;
    puq::Result res1(4.36, 0.16);
    puq::Result res2(2.35, 0.04);
    res0 = puq::math::pow(res1, 2.35);         // with a float exponent
    EXPECT_EQ(res0.to_string(), "3.18(27)e1"); // est 31.826820135086383 unc 2.74469829832924
    res0 = puq::math::pow(res1, res2);         // with an result expoenent
    EXPECT_EQ(res0.to_string(), "3.18(33)e1"); // est 31.826820135086383 unc 3.323756901862083
  }
  {
    // Measurement
    puq::Measurement msr0;
    puq::Measurement msr1(2.35, 0.04, "km2");
    puq::Measurement msr2(3.23, 0.07);
    puq::ExponentVariant exp(puq::Exponent(1, 2));
    msr0 = puq::math::pow(msr1, 2);
    EXPECT_EQ(msr0.to_string(), "5.52(19)*km4"); // est 5.522500000000001 unc 0.18800000347596324
    msr0 = puq::math::pow(msr1, exp);
    EXPECT_EQ(msr0.to_string(), "1.533(13)*km"); // est 1.5329709716755893 unc 0.013046561253560185
    msr0 = puq::math::pow(msr1, msr2);
    EXPECT_EQ(msr0.to_string(), "1.58(13)e1*km2"); // est 15.79607529212455 unc 1.2832583987008226
    try {
      puq::math::pow(msr1, msr1);
      FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
      EXPECT_STREQ(e.what(), "Exponent in the power function must be dimensionless quantity.");
    } catch (...) {
      FAIL() << "Expected std::runtime_error";
    }
  }
  {
    // Quantity   TODO: implement tests with different systems
    puq::Quantity quant0;
    puq::Quantity quant1(2.34e4, 56, "m2");
    puq::Quantity quant2(2.1, 0.1);
    quant0 = puq::math::pow(quant1, 2);
    EXPECT_EQ(quant0.to_string(), "5.476(26)e8*m4"); // est 547560000 unc 2620820.999145508
    quant0 = puq::math::pow(quant1, quant2);
    EXPECT_EQ(quant0.to_string(), "1.5(15)e9*m2"); // est 1497453345.9230108 unc 1506531188.1869695
  }
}

TEST(Math, Exponent) {

  {
    // Result
    puq::Result res0;
    puq::Result res1(2.35, 0.04);
    res0 = puq::math::exp(res1);
    EXPECT_EQ(res0.to_string(), "1.049(42)e1"); // est 10.485569724727576 unc 0.41942280901707824
  }
  {
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
      EXPECT_STREQ(e.what(), "Exponential function accepts only dimensionless quantities.");
    } catch (...) {
      FAIL() << "Expected std::runtime_error";
    }
  }
  {
    // Quantity    TODO: implement tests with different systems
    puq::Quantity quant0;
    puq::Quantity quant1(2.35, 0.04);
    quant0 = puq::math::exp(quant1);
    EXPECT_EQ(quant0.to_string(), "1.049(42)e1");
  }
}

TEST(Math, LogarithmNatural) {

  {
    // Result
    puq::Result res0;
    puq::Result res1(4.36, 0.16);
    res0 = puq::math::log(res1);
    EXPECT_EQ(res0.to_string(), "1.472(37)"); // est 1.472472057360943 unc 0.03669724719657097
  }
  {
    // Measurement
    puq::Measurement msr0;
    puq::Measurement msr1(7.23, 0.07);
    puq::Measurement msr2(2.35, 0.04, "km3");
    msr0 = puq::math::log(msr1);
    EXPECT_EQ(msr0.to_string(), "1.9782(97)"); // est 1.9782390361706734 unc 0.009681881008027917
    try {
      puq::math::log(msr2);
      FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
      EXPECT_STREQ(e.what(), "Natural logarithm accepts only dimensionless quantities.");
    } catch (...) {
      FAIL() << "Expected std::runtime_error";
    }
  }
  {
    // Quantity    TODO: implement tests with different systems
    puq::Quantity quant0;
    puq::Quantity quant1(2.35, 0.04);
    quant0 = puq::math::log(quant1);
    EXPECT_EQ(quant0.to_string(), "8.54(17)e-1"); // est 0.8544153281560676 unc 0.01702127621072691
  }
}

TEST(Math, LogarithmDecadic) {

  {
    // Result
    puq::Result res0;
    puq::Result res1(4.36, 0.16);
    res0 = puq::math::log10(res1);
    EXPECT_EQ(res0.to_string(), "6.39(16)e-1"); // est 0.6394864892685861 unc 0.01593741192351672
  }
  {
    // Measurement
    puq::Measurement msr0;
    puq::Measurement msr1(7.23, 0.07);
    puq::Measurement msr2(2.35, 0.04, "km3");
    msr0 = puq::math::log10(msr1);
    EXPECT_EQ(msr0.to_string(), "8.591(42)e-1"); // est 0.8591382972945308 unc 0.00420478755147613
    try {
      puq::math::log10(msr2);
      FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
      EXPECT_STREQ(e.what(), "Decadic logarithm accepts only dimensionless quantities.");
    } catch (...) {
      FAIL() << "Expected std::runtime_error";
    }
  }
  {
    // Quantity    TODO: implement tests with different systems
    puq::Quantity quant0;
    puq::Quantity quant1(2.35, 0.04);
    quant0 = puq::math::log10(quant1);
    EXPECT_EQ(quant0.to_string(), "3.711(74)e-1"); // est 0.37106786227173627 unc 0.0073922463261766325
  }
}

TEST(Math, CubicRoot) {

  {
    // Result
    puq::Result res0;
    puq::Result res1(4.36, 0.16);
    res0 = puq::math::cbrt(res1);
    EXPECT_EQ(res0.to_string(), "1.634(20)"); // est 1.633661834060757 unc 0.019983631105446875
  }
  {
    // Measurement
    puq::Measurement msr0;
    puq::Measurement msr1(2.35, 0.04, "km3");
    msr0 = puq::math::cbrt(msr1);
    EXPECT_EQ(msr0.to_string(), "1.3295(75)*km"); // est 1.3295028952345866 unc 0.0075432788015916685
  }
  {
    // Quantity    TODO: implement tests with different systems
    puq::Quantity quant0;
    puq::Quantity quant1(2.35, 0.04);
    quant0 = puq::math::cbrt(quant1);
    EXPECT_EQ(quant0.to_string(), "1.3295(75)"); // est 1.3295028952345866 unc 0.007543278712773827
  }
}

TEST(Math, SquareRoot) {

  {
    // Result
    puq::Result res0;
    puq::Result res1(4.36, 0.16);
    res0 = puq::math::sqrt(res1);
    EXPECT_EQ(res0.to_string(), "2.088(38)"); // est 2.08806130178211 unc 0.03831305122048434
  }
  {
    // Measurement
    puq::Measurement msr0;
    puq::Measurement msr1(4.36, 0.16, "m2");
    msr0 = puq::math::sqrt(msr1);
    EXPECT_EQ(msr0.to_string(), "2.088(38)*m"); // est 2.08806130178211 unc 0.03831305122048434
  }
  {
    // Quantity    TODO: implement tests with different systems
    puq::Quantity quant0;
    puq::Quantity quant1(2.35, 0.04);
    quant0 = puq::math::sqrt(quant1);
    EXPECT_EQ(quant0.to_string(), "1.533(13)"); // est 1.5329709716755893 unc 0.013046561253560185
  }
}

TEST(Math, Sinus) {

  {
    // Result
    puq::Result res0;
    puq::Result res1(4.36, 0.16);
    res0 = puq::math::sin(res1);
    EXPECT_EQ(res0.to_string(), "-9.39(55)e-1"); // est 0.9385508568851079 unc 0.055222547779010256
  }
  {
    // Measurement
    puq::Measurement msr0;
    puq::Measurement msr1(7.23, 0.07);
    puq::Measurement msr2(2.35, 0.04, "km3");
    msr0 = puq::math::sin(msr1);
    EXPECT_EQ(msr0.to_string(), "8.12(41)e-1"); // est 0.8115585420741488 unc 0.040898975084413536
    try {
      puq::math::sin(msr2);
      FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) { // TODO: should accept also radians
      EXPECT_STREQ(e.what(), "Sinus function accepts only dimensionless quantities.");
    } catch (...) {
      FAIL() << "Expected std::runtime_error";
    }
  }
  {
    // Quantity    TODO: implement tests with different systems
    puq::Quantity quant0;
    puq::Quantity quant1(2.35, 0.04);
    quant0 = puq::math::sin(quant1);
    EXPECT_EQ(quant0.to_string(), "7.11(28)e-1"); // est 0.7114733527908443 unc 0.02810852444135037
  }
}

TEST(Math, Cosinus) {

  {
    // Result
    puq::Result res0;
    puq::Result res1(4.36, 0.16);
    res0 = puq::math::cos(res1);
    EXPECT_EQ(res0.to_string(), "-3.5(15)e-1"); // est -0.3451409698083231 unc 0.15016814032264847
  }
  {
    // Measurement
    puq::Measurement msr0;
    puq::Measurement msr1(7.23, 0.07);
    puq::Measurement msr2(2.35, 0.04, "km3");
    msr0 = puq::math::cos(msr1);
    EXPECT_EQ(msr0.to_string(), "5.84(57)e-1"); // est 0.5842711124011541 unc 0.0568091001573734
    try {
      puq::math::cos(msr2);
      FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) { // TODO: should accept also radians
      EXPECT_STREQ(e.what(), "Cosinus function accepts only dimensionless quantities.");
    } catch (...) {
      FAIL() << "Expected std::runtime_error";
    }
  }
  {
    // Quantity    TODO: implement tests with different systems
    puq::Quantity quant0;
    puq::Quantity quant1(2.35, 0.04);
    quant0 = puq::math::cos(quant1);
    EXPECT_EQ(quant0.to_string(), "-7.03(28)e-1"); // est -0.702713076773554 unc 0.028458932632702272
  }
}

TEST(Math, Tangens) {

  {
    // Result
    puq::Result res0;
    puq::Result res1(4.36, 0.16);
    res0 = puq::math::tan(res1);
    EXPECT_EQ(res0.to_string(), "2.7(13)"); // est 2.7193261275424354 unc 1.3431579027667342
  }
  {
    // Measurement
    puq::Measurement msr0;
    puq::Measurement msr1(7.23, 0.07);
    puq::Measurement msr2(2.35, 0.04, "km3");
    msr0 = puq::math::tan(msr1);
    EXPECT_EQ(msr0.to_string(), "1.39(21)"); // est 1.3890102126372825 unc 0.20505448494745337
    try {
      puq::math::tan(msr2);
      FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) { // TODO: should accept also radians
      EXPECT_STREQ(e.what(), "Tangens function accepts only dimensionless quantities.");
    } catch (...) {
      FAIL() << "Expected std::runtime_error";
    }
    // TODO: Implement test with limiting values
  }
  {
    // Quantity    TODO: implement tests with different systems
    puq::Quantity quant0;
    puq::Quantity quant1(2.35, 0.04);
    quant0 = puq::math::tan(quant1);
    EXPECT_EQ(quant0.to_string(), "-1.012(81)"); // est -1.0124663625978223 unc 0.08100351713835607
  }
}

TEST(Math, AbsoluteValue) {

  {
    // Result
    puq::Result res0;
    puq::Result res1(-4.36, 0.16);
    res0 = puq::math::abs(res1);
    EXPECT_EQ(res0.to_string(), "4.36(16)");
  }
  {
    // Measurement
    puq::Measurement msr0;
    puq::Measurement msr1(-4.36, 0.16, "m2");
    msr0 = puq::math::abs(msr1);
    EXPECT_EQ(msr0.to_string(), "4.36(16)*m2");
  }
  {
    // Quantity    TODO: implement tests with different systems
    puq::Quantity quant0;
    puq::Quantity quant1(-4.36, 0.16, "m");
    quant0 = puq::math::abs(quant1);
    EXPECT_EQ(quant0.to_string(), "4.36(16)*m");
  }
}

TEST(Math, Maximum) {

  {
    // Result
    puq::Result res0;
    puq::Result res1(4.36, 0.16);
    puq::Result res2(2.35, 0.04);
    res0 = puq::math::max(res1, res2);
    EXPECT_EQ(res0.to_string(), "4.36(16)");
  }
  {
    // Measurement
    puq::Measurement msr0;
    puq::Measurement msr1(2.35, 0.04, "km");
    puq::Measurement msr2(3.45, 0.3, "m");
    msr0 = puq::math::max(msr1, msr2);
    EXPECT_EQ(msr0.to_string(), "2.350(40)*km");
  }
  {
    // Quantity    TODO: implement tests with different systems
    puq::Quantity quant0;
    puq::Quantity quant1(2.35, 0.04, "km");
    puq::Quantity quant2(3.45, 0.3, "m");
    quant0 = puq::math::max(quant1, quant2);
    EXPECT_EQ(quant0.to_string(), "2.350(40)*km");
  }
}

TEST(Math, Minimum) {

  {
    // Result
    puq::Result res0;
    puq::Result res1(4.36, 0.16);
    puq::Result res2(2.35, 0.04);
    res0 = puq::math::min(res1, res2);
    EXPECT_EQ(res0.to_string(), "2.350(40)");
  }
  {
    // Measurement
    puq::Measurement msr0;
    puq::Measurement msr1(2.35, 0.04, "km");
    puq::Measurement msr2(3.45, 0.3, "m");
    msr0 = puq::math::min(msr1, msr2);
    EXPECT_EQ(msr0.to_string(), "3.45(30)e-3*km");
  }
  {
    // Quantity    TODO: implement tests with different systems
    puq::Quantity quant0;
    puq::Quantity quant1(2.35, 0.04, "km");
    puq::Quantity quant2(3.45, 0.3, "m");
    quant0 = puq::math::min(quant1, quant2);
    EXPECT_EQ(quant0.to_string(), "3.45(30)e-3*km");
  }
}

TEST(Math, Floor) {

  {
    // Result
    puq::Result res0;
    puq::Result res1(4.36, 0.16);
    res0 = puq::math::floor(res1);
    EXPECT_EQ(res0.to_string(), "4");
  }
  {
    // Measurement
    puq::Measurement msr0;
    puq::Measurement msr1(4.36, 0.16, "m2");
    msr0 = puq::math::floor(msr1);
    EXPECT_EQ(msr0.to_string(), "4*m2");
  }
  {
    // Quantity    TODO: implement tests with different systems
    puq::Quantity quant0;
    puq::Quantity quant1(2.35, 0.04);
    quant0 = puq::math::floor(quant1);
    EXPECT_EQ(quant0.to_string(), "2");
  }
}

TEST(Math, Ceil) {

  {
    // Result
    puq::Result res0;
    puq::Result res1(4.36, 0.16);
    res0 = puq::math::ceil(res1);
    EXPECT_EQ(res0.to_string(), "5");
  }
  {
    // Measurement
    puq::Measurement msr0;
    puq::Measurement msr1(4.36, 0.16, "m2");
    msr0 = puq::math::ceil(msr1);
    EXPECT_EQ(msr0.to_string(), "5*m2");
  }
  {
    // Quantity    TODO: implement tests with different systems
    puq::Quantity quant0;
    puq::Quantity quant1(2.35, 0.04);
    quant0 = puq::math::ceil(quant1);
    EXPECT_EQ(quant0.to_string(), "3");
  }
}

TEST(Math, Round) {

  {
    // Result
    puq::Result res0;
    puq::Result res1(4.36, 0.16);
    puq::Result res2(4.49, 0.01);
    puq::Result res3(4.49, 0.001);
    res0 = puq::math::round(res1);
    EXPECT_EQ(res0.to_string(), "4.00(50)");
    res0 = puq::math::round(res2);
    EXPECT_EQ(res0.to_string(), "4.00(50)");
    res0 = puq::math::round(res3);
    EXPECT_EQ(res0.to_string(), "4");
  }
  {
    // Measurement
    puq::Measurement msr0;
    puq::Measurement msr1(4.36, 0.16, "m2");
    msr0 = puq::math::round(msr1);
    EXPECT_EQ(msr0.to_string(), "4.00(50)*m2");
  }
  {
    // Quantity    TODO: implement tests with different systems
    puq::Quantity quant0;
    puq::Quantity quant1(2.35, 0.04);
    quant0 = puq::math::round(quant1);
    EXPECT_EQ(quant0.to_string(), "2");
  }
}
