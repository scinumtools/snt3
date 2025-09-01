#include "pch_tests.h"

using namespace snt;

TEST(UnitSolver, Initialization) {

  puq::UnitSolver solver;
}

TEST(UnitSolver, Solve) {

  puq::UnitSolver solver;

  puq::UnitAtom atom = solver.solve(""); // empty string
  EXPECT_EQ(atom.value.to_string(), "1");

  atom = solver.solve("3*(2.0e1/5.0)"); // only numbers
  EXPECT_EQ(atom.value.to_string(), "12");

  atom = solver.solve("2.4*km/s");
  EXPECT_EQ(atom.value.to_string(), "2.4*km*s-1"); // simple arithmetics

  atom = solver.solve("3.2e4*kg*m*m/s/s"); // exponent reduction
  EXPECT_EQ(atom.value.to_string(), "3.2e4*kg*m2*s-2");

  atom = solver.solve("kg/(m*s)"); // simple parentheses
  EXPECT_EQ(atom.value.to_string(), "kg*m-1*s-1");

  atom = solver.solve("kg/(m*s)2*C"); // parentheses with eponents
  EXPECT_EQ(atom.value.to_string(), "kg*m-2*s-2*C");

  atom = solver.solve("(kg*s/(m2*K))2"); // nested parentheses with an exponent
  EXPECT_EQ(atom.value.to_string(), "kg2*s2*m-4*K-2");

  atom = solver.solve("kg*<v>2"); // quantities
  EXPECT_EQ(atom.value.to_string(), "kg*<v>2");
}

#ifdef EXPONENT_FRACTIONS

TEST(UnitSolver, SolveFractions) {

  puq::UnitSolver solver;
  puq::UnitAtom atom = solver.solve("km-2:3"); // units with fractions
  EXPECT_EQ(atom.value.to_string(), "km-2:3");

  atom = solver.solve("m1:2*m-3:5"); // reduction of fractions
  EXPECT_EQ(atom.value.to_string(), "m-1:10");

  atom = solver.solve("kg/(m*s)-1:2*C"); // parentheses with fractions
  EXPECT_EQ(atom.value.to_string(), "kg*m1:2*s1:2*C");

  atom = solver.solve("(kg3*s4/(m2*K))1:2"); // nested parentheses with an exponent
  EXPECT_EQ(atom.value.to_string(), "kg3:2*s2*m-1*K-1:2");
}

#endif

#if defined(MAGNITUDE_ARRAYS) || defined(MAGNITUDE_VALUES)

TEST(UnitSolver, SolveArrays) {

  puq::UnitSolver solver;
  puq::UnitAtom atom = solver.solve("{20, 40.5, 6.8e1}"); // numerical array only
  EXPECT_EQ(atom.value.to_string(), "{20, 40.5, ...}");

  atom = solver.solve("{20, 40.5, 6.8e1}*kg/s"); // numerical array with units
  EXPECT_EQ(atom.value.to_string(), "{20, 40.5, ...}*kg*s-1");

  atom = solver.solve("{20, 40.5, 6.8e1}*2"); // multiplied by a scalar
  EXPECT_EQ(atom.value.to_string(), "{40, 81, ...}");

  atom = solver.solve("{20, 40.5}*{2,3}"); // multiplication of vectors
  EXPECT_EQ(atom.value.to_string(), "{40, 121.5}");
}

#endif

#ifdef MAGNITUDE_ERRORS

TEST(UnitSolver, SolveErrors) {

  puq::UnitSolver solver;
  puq::UnitAtom atom = solver.solve("[#m_p]");

  puq::UnitValue uv = atom.value.convert(puq::Format::Base::MKS);
  EXPECT_EQ(uv.to_string(), "1.67262192595(52)e-27*kg");

  puq::Dimensions dim = atom.value.baseunits.dimensions();
  EXPECT_EQ(dim.to_string(), "1.67262192595(52)e-24*g");
}

#endif
