#include "pch_tests.h"

#include <snt/puq/exponent.h>
#include <snt/puq/quantity.h>
#include <snt/puq/unit_format.h>
#include <snt/puq/value/base_units.h>
#include <snt/puq/value/dimensions.h>
#include <snt/puq/value/measurement.h>

using namespace snt;

TEST(UnitFormat, ExponentFormat) {

  puq::UnitFormat format(puq::Format::Math::UNICODE);

  puq::Exponent e = puq::Exponent(-2);
  EXPECT_EQ(e.to_string(format), "⁻²"); // integer exponents

  e = puq::Exponent(3, 2);
  EXPECT_EQ(e.to_string(format), "³ᐟ²"); // positive fractional exponents

  e = puq::Exponent(-3, 2);
  EXPECT_EQ(e.to_string(format), "⁻³ᐟ²"); // negative fractional exponents
}

TEST(UnitFormat, DimensionsFormat) {

  puq::UnitFormat format;
  puq::Dimensions d(2.340342349349823e6, {1, 2, 3, 0, 0, 0, 0, 0});

  format = puq::UnitFormat(puq::Format::Precision(10)); // increased output precision
  EXPECT_EQ(d.to_string(format), "2.340342349e6*m*g2*s3");

  format = puq::UnitFormat(puq::Format::Math::UNICODE); // UNICODE math format
  EXPECT_EQ(d.to_string(format), "2.34034×10⁶⋅m⋅g²⋅s³");

  format = puq::UnitFormat(puq::Format::Display::MAGNITUDE); // display only magnitudes
  EXPECT_EQ(d.to_string(format), "2.34034e6");

  format = puq::UnitFormat(puq::Format::Display::UNITS); // display only units
  EXPECT_EQ(d.to_string(format), "m*g2*s3");

  format = puq::UnitFormat(puq::Format::Base::CGS); // change unit base
  EXPECT_EQ(d.to_string(format), "2.34034e8*cm*g2*s3");

  format = puq::UnitFormat({// change unit base and show only magnitude
                            puq::Format::Base::CGS,
                            puq::Format::Display::MAGNITUDE});
  EXPECT_EQ(d.to_string(format), "2.34034e8");

  format = puq::UnitFormat({// change unit base and show only units
                            puq::Format::Base::CGS,
                            puq::Format::Display::UNITS});
  EXPECT_EQ(d.to_string(format), "cm*g2*s3");
}

TEST(UnitFormat, BaseUnitsFormat) {

  puq::UnitFormat format(puq::Format::Math::UNICODE);
  puq::BaseUnits bus;

  bus = puq::BaseUnits("kg*m2/s2"); // integer exponents
  EXPECT_EQ(bus.to_string(format), "kg⋅m²⋅s⁻²");

  bus = puq::BaseUnits("m*kg-2:3"); // fractional exponents
  EXPECT_EQ(bus.to_string(format), "m⋅kg⁻²ᐟ³");
}

TEST(UnitFormat, MeasurementFormatFloat) {

  puq::Measurement msr;
  puq::UnitFormat format;

  format = puq::UnitFormat(puq::Format::Math::UNICODE);
  msr = puq::Measurement(3.234019394939e12); // without uncertainties
  EXPECT_EQ(msr.to_string(format), "3.23402×10¹²");

  format = puq::UnitFormat(puq::Format::Precision(10));
  msr = puq::Measurement(3.2340342349349823e3); // increased precision
  EXPECT_EQ(msr.to_string(format), "3.234034235e3");

  format = puq::UnitFormat(puq::Format::Math::UNICODE);
  msr = puq::Measurement(3.234019394939e12, 2.34e8); // with uncertainties
  EXPECT_EQ(msr.to_string(format), "3.23402(23)×10¹²");

  format = puq::UnitFormat(puq::Format::Math::UNICODE, puq::Format::Uncertainty::HIDE);
  msr = puq::Measurement(3.234019394939e12, 2.34e8); // without uncertainties
  EXPECT_EQ(msr.to_string(format), "3.23402×10¹²");
}

TEST(UnitFormat, MeasurementFormatString) {

  puq::Measurement msr;
  puq::UnitFormat format;

  format = puq::UnitFormat(puq::Format::Math::UNICODE);
  msr = puq::Measurement("3*km-2*m/s2"); // normal units
  EXPECT_EQ(msr.to_string(format), "3⋅km⁻²⋅m⋅s⁻²");

  msr = puq::Measurement("3.234019394939e12*km-2"); // decimal orders
  EXPECT_EQ(msr.to_string(format), "3.23402×10¹²⋅km⁻²");

  format = puq::UnitFormat(puq::Format::Precision(10));
  msr = puq::Measurement("3.2340342349349823e3*m/s2"); // higher precision
  EXPECT_EQ(msr.to_string(format), "3.234034235e3*m*s-2");

  format = puq::UnitFormat(puq::Format::Math::UNICODE, 10); // precision and UNICODE format
  msr = puq::Measurement("3.2340342349349823e3*km-2");
  EXPECT_EQ(msr.to_string(format), "3.234034235×10³⋅km⁻²");

  format = puq::UnitFormat(puq::Format::Display::MAGNITUDE); // display only magnitude
  msr = puq::Measurement("3*km/s");
  EXPECT_EQ(msr.to_string(format), "3");

  format = puq::UnitFormat(puq::Format::Display::UNITS); // display only magnitude
  msr = puq::Measurement("3*km/s");
  EXPECT_EQ(msr.to_string(format), "km*s-1");
}

TEST(UnitFormat, SystemFormat) {

  puq::UnitFormat format(puq::Format::System::SHOW);
  puq::Quantity q("23*cm2");
  EXPECT_EQ(q.to_string(format), "SI_23*cm2");

  format = puq::UnitFormat(puq::Format::System::SHOW, puq::Format::Math::UNICODE);
  q = puq::Quantity("23*ft2", puq::SystemType::US);
  EXPECT_EQ(q.to_string(format), "US 23⋅ft²");
}

TEST(UnitFormat, PreprocessSystem) {
  if constexpr (!puq::Config::preprocess_system) {
    GTEST_SKIP() << "PreprocessSystem is disabled";
    return;
  }

  puq::Quantity q1, q2;

  q1 = puq::Quantity(34, "ESU_statA"); // state the unit system in the unit expression
  q2 = q1.convert("Fr/ms", puq::SystemType::ESU);
  EXPECT_EQ(q2.to_string(), "0.034*Fr*ms-1");

  q1 = puq::Quantity(34, "US ft"); // same but without the underscore
  EXPECT_EQ(q1.to_string(), "34*ft");
  EXPECT_EQ(q1.unit_system(), "US");
}

TEST(UnitFormat, PreprocessSymbols) {
  if constexpr (!puq::Config::preprocess_symbols) {
    GTEST_SKIP() << "PreprocessSymbols is disabled";
    return;
  }

  puq::Quantity q;

  q = puq::Quantity("6.23537×10−12*C4⋅m4⋅J−3"); // symbols
  EXPECT_EQ(q.to_string(), "6.23537e-12*C4*m4*J-3");

  q = puq::Quantity("6.23537×1012*C4⋅m4⋅J−3");
  EXPECT_EQ(q.to_string(), "6.23537e12*C4*m4*J-3");

  q = puq::Quantity("6.23537×10⁻²⋅C³"); // exponents
  EXPECT_EQ(q.to_string(), "0.0623537*C3");
}
