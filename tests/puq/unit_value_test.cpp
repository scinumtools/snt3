#include "pch_tests.h"
#include <snt/puq/value/unit_value.h>
#include <snt/puq/value/base_units.h>
#include <snt/puq/value/dimensions.h>
#include <snt/puq/converter.h>
#include <snt/puq/exceptions.h>

using namespace snt;

TEST(UnitValue, Initialization) {

  puq::UnitValue value;

  value = {3.34e3, {{"k", "m", -1}}}; // list initialization
  EXPECT_EQ(value.to_string(), "3.34e3*km-1");

  value = puq::UnitValue(3.34e3, "km-1"); // assigning UnitValue
  EXPECT_EQ(value.to_string(), "3.34e3*km-1");

  puq::BaseUnits bu("km-1/s2");
  value = puq::UnitValue(3, bu); // from BaseUnits
  EXPECT_EQ(value.to_string(), "3*km-1*s-2");

  value = puq::UnitValue("3*km/s"); // from a string
  EXPECT_EQ(value.to_string(), "3*km*s-1");

  puq::Dimensions dim(23, {1, 2, 3, 4, 0, 0, 0, 0}); // from Dimensions
  value = puq::UnitValue(2, dim);
  EXPECT_EQ(value.to_string(), "46*m*g2*s3*K4");

#if defined(MAGNITUDE_VALUES)

  val::BaseValue::PointerType val = val::ArrayValue<int>::pointer_from_vector({2, 3, 4, 5}); // from Array
  value = puq::UnitValue(std::move(val), "km");
  EXPECT_EQ(value.to_string(), "[2, 3, 4, 5]*km");

#endif

  value = puq::UnitValue("3*km/s");
  std::stringstream ss;
  ss << value; // cast unit value as a string into a stream
  EXPECT_EQ(ss.str(), "3*km*s-1");
}

TEST(UnitValue, Size) {

  val::BaseValue::PointerType val = val::ArrayValue<int>::pointer_from_vector({2, 3, 4, 5});
  puq::UnitValue uv(std::move(val));
  EXPECT_EQ(uv.size(), 4);
}

TEST(UnitValue, RebasePrefixes) {

  puq::UnitValue value;

  value = puq::UnitValue("m*m"); // no prefixes
  value = value.rebase_prefixes();
  EXPECT_EQ(value.to_string(), "m2");

  value = puq::UnitValue("cm*cm"); // same prefixes
  value = value.rebase_prefixes();
  EXPECT_EQ(value.to_string(), "cm2");

  value = puq::UnitValue("cm*m"); // different prefixes
  value = value.rebase_prefixes();
  EXPECT_EQ(value.to_string(), "100*cm2");

  value = puq::UnitValue("m*cm");
  value = value.rebase_prefixes();
  EXPECT_EQ(value.to_string(), "0.01*m2");

  value = puq::UnitValue("mm*km");
  value = value.rebase_prefixes();
  EXPECT_EQ(value.to_string(), "1e6*mm2");

  value = puq::UnitValue("km*mm");
  value = value.rebase_prefixes();
  EXPECT_EQ(value.to_string(), "1e-6*km2");

  value = puq::UnitValue("cm2*m3"); // exponents
  value = value.rebase_prefixes();
  EXPECT_EQ(value.to_string(), "1e6*cm5");

  value = puq::UnitValue("mm2*km3");
  value = value.rebase_prefixes();
  EXPECT_EQ(value.to_string(), "1e18*mm5");

  value = puq::UnitValue("km*mm2*cm3"); // multiple units
  value = value.rebase_prefixes();
  EXPECT_EQ(value.to_string(), "1e-27*km6");

  value = puq::UnitValue("km*mm2*mg/kg2"); // with other units
  value = value.rebase_prefixes();
  EXPECT_EQ(value.to_string(), "1e-24*mg-1*km3");
}

TEST(UnitValue, RebaseDimensions) {

  puq::UnitValue value;

  value = puq::UnitValue("km*au2*pc"); // two different units of length
  value = value.rebase_dimensions();
  EXPECT_EQ(value.to_string(), "6.90565e29*km4");

  value = puq::UnitValue("kph2*s3/(h*{c}3)"); // multiple dimensions
  value = value.rebase_dimensions();
  EXPECT_EQ(value.to_string(), "2.20967e-31*kph-1*s2");

  value = puq::UnitValue("km*au*K/Cel"); // rebasing temperature units
  EXPECT_THROW(value.rebase_dimensions(), puq::UnitValueExcept);

  value = puq::UnitValue("km*au*dB/B"); // rebasing logarithmic units
  EXPECT_THROW(value.rebase_dimensions(), puq::UnitValueExcept);
}

TEST(UnitValue, InitializationFractions) {

  puq::UnitValue value1 = {3.34e3, {{"k", "m", -1, 2}}};
  EXPECT_EQ(value1.to_string(), "3.34e3*km-1:2");

  puq::UnitValue value2(3.34e3, "km-1:2");
  EXPECT_EQ(value2.to_string(), "3.34e3*km-1:2");

  puq::BaseUnits bu("km-1:2/s2");
  puq::UnitValue value3(3, bu);
  EXPECT_EQ(value3.to_string(), "3*km-1:2*s-2");
}

TEST(UnitValue, InitializationErrors) {

  puq::UnitValue v = puq::UnitValue(1.23, 0.01, "km3"); // magnitude, errors and dimensions
  EXPECT_EQ(v.to_string(), "1.230(10)*km3");

  v = puq::UnitValue(1.23, 0.01, "2*km3"); // magnitude, errors and dimensions with a number
  EXPECT_EQ(v.to_string(), "2.460(20)*km3");

  v = puq::UnitValue("3.40(10)*km3"); // unit expression
  EXPECT_EQ(v.to_string(), "3.40(10)*km3");

#if defined(MAGNITUDE_VALUES)

  val::BaseValue::PointerType am = val::ArrayValue<double>::pointer_from_vector({2, 3, 4, 5});
  val::BaseValue::PointerType ae = val::ArrayValue<double>::pointer_from_vector({0.2, 0.3, 0.4, 0.5});
  v = puq::UnitValue(std::move(am), std::move(ae), "km");
  EXPECT_EQ(v.to_string(), "[2.00(20), 3.00(30), 4.00(40), 5.00(50)]*km");

#endif
}

TEST(UnitValue, UnitConversion) {

  puq::UnitValue v1, v2, v3;

  v1 = puq::UnitValue("9*cm2");
  v2 = puq::UnitValue("3*m2");
  v3 = v1.convert(v2); // conversion using UnitValue
  EXPECT_EQ(v3.to_string(), "3e-4*m2");

  puq::BaseUnits bu("m2");
  v1 = puq::UnitValue("4*cm2");
  v2 = v1.convert(bu); // conversion using BaseUnits
  EXPECT_EQ(v2.to_string(), "4e-4*m2");

  v1 = puq::UnitValue("4*cm2");
  v2 = v1.convert("2*m2"); // conversion using an expression
  EXPECT_EQ(v2.to_string(), "2e-4*m2");

  v1 = puq::UnitValue("2*J"); // conversion to system base units
  v2 = v1.convert(puq::Format::Base::MGS);
  EXPECT_EQ(v2.to_string(), "2e3*m2*g*s-2"); // code base units
  v2 = v1.convert(puq::Format::Base::MKS);
  EXPECT_EQ(v2.to_string(), "2*m2*kg*s-2"); // SI
  v2 = v1.convert(puq::Format::Base::CGS);
  EXPECT_EQ(v2.to_string(), "2e7*cm2*g*s-2"); // CGS

  if constexpr (puq::Config::use_system_eus) {
    // conversion to foot/pound/second
    puq::UnitSystem us(puq::SystemType::US);
    v1 = puq::UnitValue("yd2*s/oz");
    v2 = v1.convert(puq::Format::Base::FPS);
    EXPECT_EQ(v2.to_string(), "144*ft2*lb-1*s");
    us.close();
  }
}

TEST(UnitValue, ArithmeticsAdd) {

  puq::UnitValue q1, q2, q3;

  q1 = puq::UnitValue(6, "cm"); // same units
  q2 = puq::UnitValue(3, "cm");
  q3 = q1 + q2;
  EXPECT_EQ(q3.to_string(), "9*cm");
  q1 += q2;
  EXPECT_EQ(q1.to_string(), "9*cm");

  q1 = puq::UnitValue(6, "cm2/s2"); // same dimensions
  q2 = puq::UnitValue(3, "m2/s2");
  q3 = q1 + q2;
  EXPECT_EQ(q3.to_string(), "3.0006e4*cm2*s-2");
  q1 += q2;
  EXPECT_EQ(q1.to_string(), "3.0006e4*cm2*s-2");

  q3 = puq::UnitValue(3, "cm2"); // different units
  EXPECT_THROW(q1 + q3, puq::ConvDimExcept);

  q1 = +puq::UnitValue(3, "cm2"); // uniary plus
  EXPECT_EQ(q1.to_string(), "3*cm2");
}

TEST(UnitValue, ArithmeticsSubtract) {

  puq::UnitValue q1, q2, q3;

  q1 = puq::UnitValue(6, "cm"); // same units
  q2 = puq::UnitValue(3, "cm");
  q3 = q1 - q2;
  EXPECT_EQ(q3.to_string(), "3*cm");
  q1 -= q2;
  EXPECT_EQ(q1.to_string(), "3*cm");

  q1 = puq::UnitValue(6, "m2/s2"); // same dimensions
  q2 = puq::UnitValue(3, "cm2/s2");
  q3 = q1 - q2;
  EXPECT_EQ(q3.to_string(), "5.9997*m2*s-2");
  q1 -= q2;
  EXPECT_EQ(q1.to_string(), "5.9997*m2*s-2");

  q3 = puq::UnitValue(3, "cm2"); // different units
  EXPECT_THROW(q1 - q3, puq::ConvDimExcept);

  q1 = -puq::UnitValue(3, "cm2"); // unary minus
  EXPECT_EQ(q1.to_string(), "-3*cm2");
}

TEST(UnitValue, ArithmeticsMultiply) {

  puq::UnitValue q1, q2, q3;

  q1 = puq::UnitValue(6, "cm3");
  q2 = puq::UnitValue(3, "g2");
  q3 = q1 * q2;
  EXPECT_EQ(q3.to_string(), "18*cm3*g2");
  q1 *= q2;
  EXPECT_EQ(q1.to_string(), "18*cm3*g2");
}

TEST(UnitValue, ArithmeticsDivide) {

  puq::UnitValue q1, q2, q3;

  q1 = puq::UnitValue(6, "cm3");
  q2 = puq::UnitValue(3, "g2");
  q3 = q1 / q2;
  EXPECT_EQ(q3.to_string(), "2*cm3*g-2");
  q1 /= q2;
  EXPECT_EQ(q1.to_string(), "2*cm3*g-2");
}

TEST(UnitValue, ArithmeticsLog) {
  if constexpr (!puq::Config::use_units_logarithmic) {
    GTEST_SKIP() << "Logarithmic units are disabled";
    return;
  }

  puq::UnitValue q1, q2, q3;

  // addition of same units
  q1 = puq::UnitValue(1, "dB");
  q2 = puq::UnitValue(2, "dB");
  q3 = q1 + q2;
  EXPECT_EQ(q3.to_string(), "4.53902*dB");
  q1 += q2;
  EXPECT_EQ(q1.to_string(), "4.53902*dB");

  // addition with different symbols, conversion
  q1 = puq::UnitValue(20, "dBm");
  q2 = puq::UnitValue(23, "dBmW");
  q3 = q1 + q2;
  EXPECT_EQ(q3.to_string(), "24.7643*dBm");
  q1 += q2;
  EXPECT_EQ(q1.to_string(), "24.7643*dBm");

  // addition of different units throws an error
  q1 = puq::UnitValue(20, "dBW");
  q2 = puq::UnitValue(23, "dBA");
  EXPECT_THROW(q1 + q2, puq::ConvDimExcept);

  // subtraction
  q1 = puq::UnitValue(87, "dBA");
  q2 = puq::UnitValue(83, "dBA");
  q3 = q1 - q2;
  EXPECT_EQ(q3.to_string(), "84.7952*dBA");
  q1 -= q2;
  EXPECT_EQ(q1.to_string(), "84.7952*dBA");
}
