#include "pch_tests.h"

using namespace snt;

TEST(UnitAtom, FromString) {

  puq::UnitValue value = puq::UnitAtom::from_string("1");
  EXPECT_EQ(value.to_string(), "1"); // unit

  value = puq::UnitAtom::from_string("2");
  EXPECT_EQ(value.to_string(), "2"); // integer numbers

  value = puq::UnitAtom::from_string("1.23");
  EXPECT_EQ(value.to_string(), "1.23"); // float numbers

  value = puq::UnitAtom::from_string("1.23e6");
  EXPECT_EQ(value.to_string(), "1.23e6"); // numbers in scientific notation

  value = puq::UnitAtom::from_string("s");
  EXPECT_EQ(value.to_string(), "s"); // units without prefixes

  value = puq::UnitAtom::from_string("mC");
  EXPECT_EQ(value.to_string(), "mC"); // allowed all prefixes

  value = puq::UnitAtom::from_string("mrad");
  EXPECT_EQ(value.to_string(), "mrad"); // allowed specific prefix

  EXPECT_THROW(puq::UnitAtom::from_string("kau"), puq::AtomParsingExcept); // prefixes are not allowed

  EXPECT_THROW(puq::UnitAtom::from_string("crad"), puq::AtomParsingExcept); // unit prefix is not allowed

  EXPECT_THROW(puq::UnitAtom::from_string("Hm"), puq::AtomParsingExcept); // unknown unit prefix

  value = puq::UnitAtom::from_string("kg2");
  EXPECT_EQ(value.to_string(), "kg2"); // units with exponents

  value = puq::UnitAtom::from_string("cm-2");
  EXPECT_EQ(value.to_string(), "cm-2"); // negative exponents
}

#ifdef MAGNITUDE_ERRORS

TEST(UnitAtom, FromStringErrors) {

  puq::UnitValue value = puq::UnitAtom::from_string("1.2345(67)"); // without exponent
  EXPECT_EQ(value.to_string(), "1.2345(67)");

  value = puq::UnitAtom::from_string("1.223(23)e+02"); // with an exponent
  EXPECT_EQ(value.to_string(), "1.223(23)e2");

  value = puq::UnitAtom::from_string("1.223(233)e+02"); // more error digits
  EXPECT_EQ(value.to_string(), "1.22(23)e2");

  value = puq::UnitAtom::from_string("1.2235(2)e+03"); // less error digits
  EXPECT_EQ(value.to_string(), "1.22350(20)e3");

  value = puq::UnitAtom::from_string("122.(23)e+03"); // no decimal digits
  EXPECT_EQ(value.to_string(), "1.22(23)e5");

  value = puq::UnitAtom::from_string("122(23)e+03"); // no decimal point
  EXPECT_EQ(value.to_string(), "1.22(23)e5");
}

#endif

#ifdef EXPONENT_FRACTIONS

TEST(UnitAtom, FromStringFractions) {

  puq::UnitValue value = puq::UnitAtom::from_string("kg4:2");
  EXPECT_EQ(value.to_string(), "kg2"); // reduced fractions

  value = puq::UnitAtom::from_string("kg-2:3");
  EXPECT_EQ(value.to_string(), "kg-2:3"); // full fractions
}

#endif

TEST(UnitAtom, Arithmetics) {

  puq::UnitAtom a1({6.0, {{"a", "b", 1}}});
  puq::UnitAtom a2({3.0, {{"c", "d", 3}}});

  a1.math_multiply(&a2); // 6 * 3 = 18
  EXPECT_EQ(a1.value.to_string(), "18*ab*cd3");

  a1.math_divide(&a2); // 18 / 3 = 6
  EXPECT_EQ(a1.value.to_string(), "6*ab");
}

#ifdef EXPONENT_FRACTIONS

TEST(UnitAtom, ArithmeticsFractions) {

  puq::UnitAtom a1({6.0, {{"c", "m", 1, 2}}});
  puq::UnitAtom a2({3.0, {{"k", "g", 3, 4}}});

  a1.math_multiply(&a2); // Atom multiplication: 6 * 3 = 18
  EXPECT_EQ(a1.value.to_string(), "18*cm1:2*kg3:4");

  a1.math_divide(&a2); // Atom division: 18 / 3 = 6
  EXPECT_EQ(a1.value.to_string(), "6*cm1:2");
}

#endif
