#include "dip/dip.h"
#include "dip/solvers/solvers.h"

#include <gtest/gtest.h>

TEST(SolverNumerical, AddSubtract) {

  dip::Environment env;
  dip::NumericalSolver solver(env);
  dip::NumericalAtom atom = solver.eval("1.23 + 4.56");
  EXPECT_EQ(atom.value->to_string(), "5.7900");

  atom = solver.eval("4.56 - 1.23");
  EXPECT_EQ(atom.value->to_string(), "3.3300");

  atom = solver.eval("1.23 + 4.56 - 7.89");
  EXPECT_EQ(atom.value->to_string(), "-2.1000");

  atom = solver.eval("+1.23");
  EXPECT_EQ(atom.value->to_string(), "1.2300");

  atom = solver.eval("-1.23");
  EXPECT_EQ(atom.value->to_string(), "-1.2300");

  atom = solver.eval("1.23 + -2.34");
  EXPECT_EQ(atom.value->to_string(), "-1.1100");
}

TEST(SolverNumerical, MultiplyDivide) {

  dip::Environment env;
  dip::NumericalSolver solver(env);
  dip::NumericalAtom atom = solver.eval("1.23 * 4.56");
  EXPECT_EQ(atom.value->to_string(), "5.6088");

  atom = solver.eval("4.56 / 1.23");
  EXPECT_EQ(atom.value->to_string(), "3.7073");

  atom = solver.eval("1.23 * 4.56 / 7.89");
  EXPECT_EQ(atom.value->to_string(), "0.7109");
}

TEST(SolverNumerical, GroupOperators) {

  dip::Environment env;
  dip::NumericalSolver solver(env);
  dip::NumericalAtom atom = solver.eval("1.23 ** 4.56");
  EXPECT_EQ(atom.value->to_string(), "2.5702");

  atom = solver.eval("exp( 1.23 )");
  EXPECT_EQ(atom.value->to_string(), "3.4212");

  atom = solver.eval("log( 19.0 )");
  EXPECT_EQ(atom.value->to_string(), "2.9444");

  atom = solver.eval("log10( 19.0 )");
  EXPECT_EQ(atom.value->to_string(), "1.2788");

  atom = solver.eval("sqrt( 9.0 )");
  EXPECT_EQ(atom.value->to_string(), "3.0000");

  atom = solver.eval("cbrt( 8.0 )");
  EXPECT_EQ(atom.value->to_string(), "2.0000");

  atom = solver.eval("sin( 8.0 )");
  EXPECT_EQ(atom.value->to_string(), "0.9894");

  atom = solver.eval("cos( 8.0 )");
  EXPECT_EQ(atom.value->to_string(), "-0.1455");

  atom = solver.eval("tan( 8.0 )");
  EXPECT_EQ(atom.value->to_string(), "-6.7997");

  atom = solver.eval("2.0 * ( 8.0 - 4.0 )");
  EXPECT_EQ(atom.value->to_string(), "8.0000");

  atom = solver.eval("cos( 2.0 * ( sqrt( 16.0 ) - 2.0 ) )");
  EXPECT_EQ(atom.value->to_string(), "-0.6536");
}

TEST(SolverNumerical, Injections) {

  dip::DIP d;
  d.add_string("foo float = 4.52");
  d.add_string("bar int = 3");
  dip::Environment env = d.parse();

  dip::NumericalSolver solver(env);
  dip::NumericalAtom atom = solver.eval("sin( {?foo} ) - 2 * {?bar}");
  EXPECT_EQ(atom.value->to_string(), "-6.9816");
}
