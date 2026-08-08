#include "pch_tests.h"

#include <memory>
#include <snt/exs/atom.h>
#include <snt/exs/solver.h>

using namespace snt;

// Basic mathematic expressions
TEST(Solver, SolvingMath) {

    exs::Solver<exs::Atom> solver;

    exs::Atom atom = solver.eval("1.2 + 34");
    EXPECT_EQ(atom.to_string(), "35.2");
    atom = solver.eval("32 - 34");
    EXPECT_EQ(atom.to_string(), "-2");
    atom = solver.eval("32 * 2");
    EXPECT_EQ(atom.to_string(), "64");
    atom = solver.eval("32.4 / 2");
    EXPECT_EQ(atom.to_string(), "16.2");
    atom = solver.eval("3 ** 3");
    EXPECT_EQ(atom.to_string(), "27");
    atom = solver.eval("19 % 3");
    EXPECT_EQ(atom.to_string(), "1");
    atom = solver.eval("1.2 + +68/2 - -2**5 * 93");
    EXPECT_EQ(atom.to_string(), "3011.2");
    atom = solver.eval("1.234e32 * 3.33");
    EXPECT_EQ(atom.to_string(), "4.10922e+32");
}

// Basic comparison expressions
TEST(Solver, SolvingComparison) {

    exs::Solver<exs::Atom> solver;

    exs::Atom atom = solver.eval("23 == 34");
    EXPECT_EQ(atom.to_string(), "false");
    atom = solver.eval("23.2 == 23.2");
    EXPECT_EQ(atom.to_string(), "true");

    atom = solver.eval("23 != 34");
    EXPECT_EQ(atom.to_string(), "true");
    atom = solver.eval("23.2 != 23.2");
    EXPECT_EQ(atom.to_string(), "false");

    atom = solver.eval("23 <= 34");
    EXPECT_EQ(atom.to_string(), "true");
    atom = solver.eval("34 <= 23");
    EXPECT_EQ(atom.to_string(), "false");
    atom = solver.eval("23 <= 23");
    EXPECT_EQ(atom.to_string(), "true");

    atom = solver.eval("23 >= 34");
    EXPECT_EQ(atom.to_string(), "false");
    atom = solver.eval("34 >= 23");
    EXPECT_EQ(atom.to_string(), "true");
    atom = solver.eval("23 >= 23");
    EXPECT_EQ(atom.to_string(), "true");

    atom = solver.eval("23 < 34");
    EXPECT_EQ(atom.to_string(), "true");
    atom = solver.eval("34.2 < 23.2");
    EXPECT_EQ(atom.to_string(), "false");

    atom = solver.eval("23 > 34");
    EXPECT_EQ(atom.to_string(), "false");
    atom = solver.eval("34.2 > 23.2");
    EXPECT_EQ(atom.to_string(), "true");
}

// Basic logical expressions
TEST(Solver, SolvingLogical) {

    exs::Solver<exs::Atom> solver;

    exs::Atom atom = solver.eval("!true");
    EXPECT_EQ(atom.to_string(), "false");
    atom = solver.eval("!false");
    EXPECT_EQ(atom.to_string(), "true");

    atom = solver.eval("false && true");
    EXPECT_EQ(atom.to_string(), "false");
    atom = solver.eval("true && true");
    EXPECT_EQ(atom.to_string(), "true");

    atom = solver.eval("false || true");
    EXPECT_EQ(atom.to_string(), "true");
    atom = solver.eval("false || false");
    EXPECT_EQ(atom.to_string(), "false");
}

// Basic group expressions
TEST(Solver, SolvingGroups) {

    exs::Solver<exs::Atom> solver;

    exs::Atom atom = solver.eval("(1+2)-3");
    EXPECT_EQ(atom.to_string(), "0");

    atom = solver.eval("exp(1+2)-3");
    EXPECT_EQ(atom.to_string(), "17.0855");

    atom = solver.eval("log(1+2)-3");
    EXPECT_EQ(atom.to_string(), "-1.90139");

    atom = solver.eval("log10(1+2)-3");
    EXPECT_EQ(atom.to_string(), "-2.52288");

    atom = solver.eval("logb(1+2,2)-3");
    EXPECT_EQ(atom.to_string(), "-1.41504");

    atom = solver.eval("powb(2,3)-3");
    EXPECT_EQ(atom.to_string(), "5");

    atom = solver.eval("sqrt(16)-3");
    EXPECT_EQ(atom.to_string(), "1");

    atom = solver.eval("cbrt(27)-2");
    EXPECT_EQ(atom.to_string(), "1");

    atom = solver.eval("sin(16)-3");
    EXPECT_EQ(atom.to_string(), "-3.2879");

    atom = solver.eval("cos(16)-3");
    EXPECT_EQ(atom.to_string(), "-3.95766");

    atom = solver.eval("tan(16)-3");
    EXPECT_EQ(atom.to_string(), "-2.69937");

    // nested group operators
    atom = solver.eval("exp(1+(5-3))-3");
    EXPECT_EQ(atom.to_string(), "17.0855");
}

// Conditional operator
TEST(Solver, SolvingCondition) {

    exs::Solver<exs::Atom> solver;

    exs::Atom atom = solver.eval("true ? 2 : 3");
    EXPECT_EQ(atom.to_string(), "2");

    atom = solver.eval("(true ? 2 : 3)-3");
    EXPECT_EQ(atom.to_string(), "-1");

    atom = solver.eval("(false ? 2 : 3)-3");
    EXPECT_EQ(atom.to_string(), "0");

    atom = solver.eval("((2==3) ? 2 : 3)-3");
    EXPECT_EQ(atom.to_string(), "0");
}

TEST(Solver, CustomStepList) {

    // create custom step list
    exs::StepList steps;
    steps.append(exs::UNARY_OPERATION, {exs::ADD_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::MULTIPLY_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::ADD_OPERATOR});

    // test the solver
    exs::Solver<exs::Atom> solver(steps);
    exs::Atom atom = solver.eval("1.2 + 5 * 4");
    EXPECT_EQ(atom.to_string(), "21.2");
}

TEST(Solver, CustomOperatorList) {

    // create custom operator list
    exs::OperatorList operators;
    operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd>());
    operators.append(exs::MULTIPLY_OPERATOR, std::make_shared<exs::OperatorMultiply>());
    operators.append(exs::POWER_OPERATOR, std::make_shared<exs::OperatorPower>("^")); // changin the symbol

    // test the solver
    exs::Solver<exs::Atom> solver(operators);
    exs::Atom atom = solver.eval("1.2 + 5 * 2^2");
    EXPECT_EQ(atom.to_string(), "21.2");
}

class OperatorParentheses : public exs::OperatorGroup<1> {
  public:
    OperatorParentheses() : OperatorGroup<1>("par", {"", "(", ")", ","}, exs::PARENTHESES_OPERATOR) {}
};

TEST(Operators, CustomParenthesesOperator) {

    exs::OperatorList operators;
    operators.append(exs::PARENTHESES_OPERATOR, std::make_shared<OperatorParentheses>());
    operators.append(exs::MULTIPLY_OPERATOR, std::make_shared<exs::OperatorMultiply>());
    operators.append(exs::DIVIDE_OPERATOR, std::make_shared<exs::OperatorDivide>());

    // create custom step list
    exs::StepList steps;
    steps.append(exs::GROUP_OPERATION, {exs::PARENTHESES_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::MULTIPLY_OPERATOR, exs::DIVIDE_OPERATOR});

    // test the solver
    exs::Solver<exs::Atom> solver(operators);
    exs::Atom atom = solver.eval("2 * (12 / 4)");
    EXPECT_EQ(atom.to_string(), "6");
}
