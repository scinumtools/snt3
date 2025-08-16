#include <gtest/gtest.h>

#include "puq/calc/calc.h"

TEST(Calculator, Initialization) {

  puq::Calculator calc;

  puq::CalculatorAtom atom = calc.solve(""); // empty string
  EXPECT_EQ(atom.value.to_string(), "1");
}

TEST(Calculator, Arithmetics) {

  puq::Calculator calc;

  puq::CalculatorAtom atom = calc.solve("12*cm + 3*m"); // addition
  EXPECT_EQ(atom.value.to_string(), "312*cm");

  atom = calc.solve("4*cm - 3*mm"); // subtration
  EXPECT_EQ(atom.value.to_string(), "3.7*cm");

  atom = calc.solve("2*cm * 3*m"); // multiplication
  EXPECT_EQ(atom.value.to_string(), "6*cm*m");
  atom.value.value = atom.value.value.rebase_prefixes();
  EXPECT_EQ(atom.value.to_string(), "600*cm2");

  atom = calc.solve("6*cm / 2*g"); // division
  EXPECT_EQ(atom.value.to_string(), "3*cm*g-1");

  atom = calc.solve("2*g * ( 6*m2/s2 - 3*cm2 / 1.5*s2 )"); // parentheses
  EXPECT_EQ(atom.value.to_string(), "11.9996*g*m2*s-2");
}

TEST(Calculator, UnitSystems) {

  puq::Calculator calc;
  puq::CalculatorAtom atom = calc.solve("SI_12*cm + US_3*ft"); // addition
  EXPECT_EQ(atom.value.to_string(), "103.44*cm");

  atom = calc.solve("IU_33*yd - SI_3*m"); // subtraction
  EXPECT_EQ(atom.value.to_string(), "29.7192*yd");

  puq::Quantity q1("SI_12*cm"), q2("US_3*ft");  // multiplication and division is not supported
  EXPECT_THROW(q1 * q2, puq::UnitSystemExcept); // because we do not know conversion units
  EXPECT_THROW(q1 / q2, puq::UnitSystemExcept);
}
