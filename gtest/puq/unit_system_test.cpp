#include "puq/converter.h"
#include "puq/quantity.h"

#include <gtest/gtest.h>

#ifdef UNIT_SYSTEM_CGS

TEST(UnitSystem, PrintUnitSystem) {

  puq::Quantity q("23*cm");
  EXPECT_EQ(q.unit_system(), "SI");

  q = puq::Quantity("23*cm", puq::SystemType::ESU);
  EXPECT_EQ(q.unit_system(), "ESU");

  puq::UnitSystem us(puq::SystemType::AU);
  q = puq::Quantity("3*E_h");
  EXPECT_EQ(q.unit_system(), "AU");
}

TEST(UnitSystem, DirectSetting) {

  puq::Quantity q1, q2;

  q1 = puq::Quantity("34*statA", puq::SystemType::ESU); // set unit system using an abbreviation
  EXPECT_EQ(q1.to_string(), "34*statA");

  q1 = puq::Quantity(34, "statA", puq::SystemType::ESU); // explicitely state the unit system
  q2 = q1.convert("Fr/ms", puq::SystemType::ESU);
  EXPECT_EQ(q2.to_string(), "0.034*Fr*ms-1");
}

TEST(UnitSystem, EnvironmentSetting) {

  puq::Quantity q1, q2;

  puq::UnitSystem us(puq::SystemType::ESU); // set a default unit system environment

  q1 = puq::Quantity(34, "statA"); // ESU units are available by default
  q2 = q1.convert("Fr/ms");
  EXPECT_EQ(q2.to_string(), "0.034*Fr*ms-1");

  us.close(); // switch to previous system
  EXPECT_THROW(us.close(), puq::UnitSystemExcept);

  EXPECT_THROW(puq::Quantity(2, "statA"), puq::AtomParsingExcept); // ESU is no more available

  q1 = puq::Quantity(2, "A");
  EXPECT_EQ(q1.to_string(), "2*A"); // SI is available
}

TEST(UnitSystem, FormatConversion) {

  puq::Quantity q("J"), r;

  r = q.convert(puq::Format::Base::MKS);
  EXPECT_EQ(r.to_string(), "m2*kg*s-2");

  r = q.convert(puq::Format::Base::MGS);
  EXPECT_EQ(r.to_string(), "1000*m2*g*s-2");

  r = q.convert(puq::Format::Base::CGS);
  EXPECT_EQ(r.to_string(), "1e+07*cm2*g*s-2");

#ifdef UNIT_SYSTEM_EUS
  // q is implicitely converted into MGS format,
  // because unit J does not exist in US unit system
  r = q.convert(puq::Format::Base::FPS, puq::SystemType::US);
  EXPECT_EQ(r.to_string(), "23.7304*ft2*lb*s-2");

  // explicitely what happens above
  r = q.convert(puq::Format::Base::MGS);
  r = r.convert(puq::Format::Base::FPS, puq::SystemType::US);
  EXPECT_EQ(r.to_string(), "23.7304*ft2*lb*s-2");
#endif
}

TEST(UnitSystem, DirectConversionESU) {

  puq::Quantity q1, q2, q3;

  q1 = puq::Quantity(34, "J", puq::SystemType::SI); // convert using expression
  q2 = q1.convert("erg", puq::SystemType::ESU);
  EXPECT_EQ(q2.to_string(), "3.4e+08*erg");

  q1 = puq::Quantity(34, "J", puq::SystemType::SI); // convert using Quantity/UnitValue
  q2 = puq::Quantity("erg", puq::SystemType::ESU);
  q3 = q1.convert(q2);
  EXPECT_EQ(q3.to_string(), "3.4e+08*erg");

#ifdef PREPROCESS_SYSTEM
  q1 = puq::Quantity(34, "J");
  q2 = q1.convert("ESU_erg");
  EXPECT_EQ(q2.to_string(), "3.4e+08*erg");
#endif
}

TEST(UnitSystem, ContextConversionESU) {

  puq::Quantity q1, q2, q3;

  q1 = puq::Quantity(20, "statA", puq::SystemType::ESU);
  q2 = q1.convert("A", puq::SystemType::SI, "I"); // specify conversion context
  EXPECT_EQ(q2.to_string(), "6.67128e-09*A");

  q1 = puq::Quantity("6.671282e-10*A", puq::SystemType::SI);
  q2 = q1.convert("statA", puq::SystemType::ESU, "I"); // inverse conversion
  EXPECT_EQ(q2.to_string(), "2*statA");

  q1 = puq::Quantity("1.13412e-08*A", puq::SystemType::SI); // error without given context
  EXPECT_THROW(q1.convert("statA", puq::SystemType::ESU), puq::ConvDimExcept);

  q1 = puq::Quantity(1, "statT", puq::SystemType::ESU);
  q2 = q1.convert("T", puq::SystemType::SI, "B"); // specify conversion context
  EXPECT_EQ(q2.to_string(), "2.99792e+06*T");

  q1 = puq::Quantity(1, "erg", puq::SystemType::ESU); // quantities that do not need a conversion context
  q2 = q1.convert("J", puq::SystemType::SI, "E");     // with a context
  EXPECT_EQ(q2.to_string(), "1e-07*J");
  q2 = q1.convert("J", puq::SystemType::SI); // without a context
  EXPECT_EQ(q2.to_string(), "1e-07*J");

#ifdef PREPROCESS_SYSTEM
  // throw an error if preprocessor system does not match selected system
  q1 = puq::Quantity("6.671282e-10*A");
  EXPECT_THROW(q1.convert("ESU_statA", puq::SystemType::SI, "I"), puq::UnitSystemExcept);
#endif
}

#endif

#ifdef UNIT_SYSTEM_NUS

TEST(UnitSystem, ContextConversionSRU) {

  puq::Quantity q1, q2, q3;

  q1 = puq::Quantity(1, "<E>", puq::SystemType::SRU);
  q2 = q1.convert("<E>", puq::SystemType::SI, "E");
  EXPECT_EQ(q2.to_string(), "8.98755e+16*<E>");

  q1 = puq::Quantity(1, "<L_ome>", puq::SystemType::SRU);
  q2 = q1.convert("<L_ome>", puq::SystemType::SI, "L_ome");
  EXPECT_EQ(q2.to_string(), "2.99792e+08*<L_ome>");
}

TEST(UnitSystem, ContextConversionGRU) {

  puq::Quantity q1, q2, q3;

  q1 = puq::Quantity(1, "<E>", puq::SystemType::GRU);
  q2 = q1.convert("<E>", puq::SystemType::SI, "E");
  EXPECT_EQ(q2.to_string(), "1.60218e-10*<E>");

  q1 = puq::Quantity(1, "<L_ome>", puq::SystemType::GRU);
  q2 = q1.convert("<L_ome>", puq::SystemType::SI, "L_ome");
  EXPECT_EQ(q2.to_string(), "1.05457e-34*<L_ome>");
}

TEST(UnitSystem, ContextConversionGEO) {

  puq::Quantity q1, q2, q3;

  q1 = puq::Quantity(1, "m*[#c]2/[#G]", puq::SystemType::GEO);
  q1 = q1.convert(puq::Format::Base::MKS);
  EXPECT_EQ(q1.to_string(), "1.346591(30)e+27*kg");

  q1 = puq::Quantity(1, "<E>", puq::SystemType::GEO);
  q2 = q1.convert("<E>", puq::SystemType::SI, "E");
  EXPECT_EQ(q2.to_string(), "1.210256(27)e+44*<E>");

  q1 = puq::Quantity(1, "<L_ome>", puq::SystemType::GEO);
  q2 = q1.convert("<L_ome>", puq::SystemType::SI, "L_ome");
  EXPECT_EQ(q2.to_string(), "4.036978(91)e+35*<L_ome>");
}

#endif
