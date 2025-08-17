#include <gtest/gtest.h>

#include "dip/solvers/solvers.h"

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

TEST(SolverNumerical, Power) {

  dip::Environment env;
  dip::NumericalSolver solver(env);
  dip::NumericalAtom atom = solver.eval("1.23 ** 4.56");
  EXPECT_EQ(atom.value->to_string(), "2.5702");

}
