#include <gtest/gtest.h>

#include "../../src/dip/solvers/solvers.h"

TEST(SolverLogical, AndOrNot) {

  dip::Environment env;
  dip::LogicalSolver solver(env);
  dip::LogicalAtom atom = solver.eval("true && false && true");
  EXPECT_EQ(atom.value->to_string(), "false");

  atom = solver.eval("true && true && true");
  EXPECT_EQ(atom.value->to_string(), "true");

  atom = solver.eval("true || true || false");
  EXPECT_EQ(atom.value->to_string(), "true");

  atom = solver.eval("false || false || false");
  EXPECT_EQ(atom.value->to_string(), "false");

  atom = solver.eval("!false");
  EXPECT_EQ(atom.value->to_string(), "true");
}

TEST(SolverLogical, BoolComparison) {

  dip::Environment env;
  dip::LogicalSolver solver(env);
  dip::LogicalAtom atom = solver.eval("true == false");
  EXPECT_EQ(atom.value->to_string(), "false");

  atom = solver.eval("true != false");
  EXPECT_EQ(atom.value->to_string(), "true");
}

TEST(SolverLogical, IntegerComparison) {

  dip::Environment env;
  dip::LogicalSolver solver(env);
  dip::LogicalAtom atom = solver.eval("2 == 3");
  EXPECT_EQ(atom.value->to_string(), "false");

  atom = solver.eval("2 != 3");
  EXPECT_EQ(atom.value->to_string(), "true");

  atom = solver.eval("2 > 3");
  EXPECT_EQ(atom.value->to_string(), "false");

  atom = solver.eval("2 < 3");
  EXPECT_EQ(atom.value->to_string(), "true");

  atom = solver.eval("2 >= 2");
  EXPECT_EQ(atom.value->to_string(), "true");

  atom = solver.eval("2 <= 2");
  EXPECT_EQ(atom.value->to_string(), "true");
}

TEST(SolverLogical, FloatComparison) {

  dip::Environment env;
  dip::LogicalSolver solver(env);
  dip::LogicalAtom atom = solver.eval("2.0 == 3.23e3");
  EXPECT_EQ(atom.value->to_string(), "false");

  atom = solver.eval("2.00e3 != 2000.");
  EXPECT_EQ(atom.value->to_string(), "false");
}

TEST(SolverLogical, StringComparison) {

  dip::Environment env;
  dip::LogicalSolver solver(env);
  dip::LogicalAtom atom = solver.eval("'foo' == 'bar'");
  EXPECT_EQ(atom.value->to_string(), "false");

  atom = solver.eval("'foo' != 'foo'");
  EXPECT_EQ(atom.value->to_string(), "false");
}

TEST(SolverLogical, CombinedExpressions) {

  dip::Environment env;
  dip::LogicalSolver solver(env);
  dip::LogicalAtom atom = solver.eval("2 == 3 && false");
  EXPECT_EQ(atom.value->to_string(), "false");

  atom = solver.eval("( true && 2 != 3 ) || false || 2.3 > 4.3");
  EXPECT_EQ(atom.value->to_string(), "true");
}
