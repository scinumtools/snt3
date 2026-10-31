#include "pch_tests.h"

#include <snt/dip/dip.h>
#include <snt/dip/environment.h>
#include <snt/dip/solvers/logical_solver.h>

using namespace snt;

TEST(SolverLogical, AndOrNot) {

    dip::Environment env;
    dip::LogicalSolver solver(env);
    dip::ValueNodeData data = solver.eval("true && false && true");
    EXPECT_EQ(data.value->to_string(), "false");

    data = solver.eval("true && true && true");
    EXPECT_EQ(data.value->to_string(), "true");

    data = solver.eval("true || true || false");
    EXPECT_EQ(data.value->to_string(), "true");

    data = solver.eval("false || false || false");
    EXPECT_EQ(data.value->to_string(), "false");

    data = solver.eval("~false");
    EXPECT_EQ(data.value->to_string(), "true");
}

TEST(SolverLogical, BoolComparison) {

    dip::Environment env;
    dip::LogicalSolver solver(env);
    dip::ValueNodeData data = solver.eval("true == false");
    EXPECT_EQ(data.value->to_string(), "false");

    data = solver.eval("true != false");
    EXPECT_EQ(data.value->to_string(), "true");
}

TEST(SolverLogical, IntegerComparison) {

    dip::Environment env;
    dip::LogicalSolver solver(env);
    dip::ValueNodeData data = solver.eval("2 == 3");
    EXPECT_EQ(data.value->to_string(), "false");

    data = solver.eval("2 != 3");
    EXPECT_EQ(data.value->to_string(), "true");

    data = solver.eval("2 > 3");
    EXPECT_EQ(data.value->to_string(), "false");

    data = solver.eval("2 < 3");
    EXPECT_EQ(data.value->to_string(), "true");

    data = solver.eval("2 >= 2");
    EXPECT_EQ(data.value->to_string(), "true");

    data = solver.eval("2 <= 2");
    EXPECT_EQ(data.value->to_string(), "true");
}

TEST(SolverLogical, FloatComparison) {

    dip::Environment env;
    dip::LogicalSolver solver(env);
    dip::ValueNodeData data = solver.eval("2.0 == 3.23e3");
    EXPECT_EQ(data.value->to_string(), "false");

    data = solver.eval("2.00e3 != 2000.");
    EXPECT_EQ(data.value->to_string(), "false");
}

TEST(SolverLogical, DimensionalComparison) {
    dip::Environment env;
    dip::LogicalSolver solver(env);
    EXPECT_EQ(solver.eval("1 s > 0 s").value->to_string(), "true");
    EXPECT_EQ(solver.eval("1 m > 50 cm").value->to_string(), "true");
}

TEST(SolverLogical, RejectsDimensionMismatchForEveryComparison) {
    dip::Environment env;
    dip::LogicalSolver solver(env);
    for (const char* operation : {"==", "!=", "<=", ">=", "<", ">"}) {
        EXPECT_THROW(solver.eval(std::string("1 s ") + operation + " 0"), dip::UnitException);
        EXPECT_THROW(solver.eval(std::string("1 ") + operation + " 0 s"), dip::UnitException);
    }
}

TEST(SolverLogical, StringComparison) {

    dip::Environment env;
    dip::LogicalSolver solver(env);
    dip::ValueNodeData data = solver.eval("\"foo\" == \"bar\"");
    EXPECT_EQ(data.value->to_string(), "false");

    data = solver.eval("\"foo\" != \"foo\"");
    EXPECT_EQ(data.value->to_string(), "false");
}

TEST(SolverLogical, Definition) {

    dip::Environment env;
    dip::LogicalSolver solver(env);
    dip::ValueNodeData data = solver.eval("def( {?foo} )");
    EXPECT_EQ(data.value->to_string(), "false");

    data = solver.eval("ndef( true )");
    EXPECT_EQ(data.value->to_string(), "false");
}

TEST(SolverLogical, NumericalPredicates) {
    dip::DIP input;
    input.add_string("nan_value float = nan\npositive_inf float = inf\nnegative_inf float = -inf");
    dip::Environment env = input.parse();
    dip::LogicalSolver solver(env);
    for (const auto& [expression, expected] : std::vector<std::pair<std::string, bool>>{
             {"has_nan({?nan_value})", true}, {"has_nan({?positive_inf})", false},
             {"has_inf({?negative_inf})", true}, {"has_pos_inf({?positive_inf})", true},
             {"has_pos_inf({?negative_inf})", false}, {"has_neg_inf({?negative_inf})", true},
             {"has_positive({?positive_inf})", true}, {"has_negative({?negative_inf})", true},
             {"has_zero(-0.0)", true}, {"has_zero(1)", false},
             {"all_finite(3 m)", true}, {"all_finite({?nan_value})", false},
             {"~has_nan(1)", true}}) {
        EXPECT_EQ(solver.eval(expression).value->to_string(), expected ? "true" : "false") << expression;
    }
    EXPECT_THROW(solver.eval("has_nan(true)"), dip::SolverException);
    EXPECT_THROW(solver.eval("has_zero(\"0\")"), dip::SolverException);
}

TEST(SolverLogical, NumericalPredicatesOnArrays) {
    dip::DIP input;
    input.add_string("samples float[3] = [1, nan, -inf]");
    input.add_string("empty float[0] = []");
    dip::Environment env = input.parse();
    dip::LogicalSolver solver(env);
    EXPECT_EQ(solver.eval("has_nan({?samples})").value->to_string(), "true");
    EXPECT_EQ(solver.eval("has_neg_inf({?samples})").value->to_string(), "true");
    EXPECT_EQ(solver.eval("has_zero({?samples})").value->to_string(), "false");
    EXPECT_EQ(solver.eval("all_finite({?samples})").value->to_string(), "false");
    EXPECT_EQ(solver.eval("has_nan({?empty})").value->to_string(), "false");
    EXPECT_EQ(solver.eval("all_finite({?empty})").value->to_string(), "true");
}

TEST(SolverLogical, CombinedExpressions) {

    dip::Environment env;
    dip::LogicalSolver solver(env);
    dip::ValueNodeData data = solver.eval("2 == 3 && false");
    EXPECT_EQ(data.value->to_string(), "false");

    data = solver.eval("( true && 2 != 3 ) || false || 2.3 > 4.3");
    EXPECT_EQ(data.value->to_string(), "true");
}

TEST(SolverLogical, References) {

    // solve expression
    dip::DIP d;
    d.add_string("foo bool = false");
    d.add_string("bar int = 3");
    dip::Environment env = d.parse();

    dip::LogicalSolver solver(env);
    dip::ValueNodeData data = solver.eval("2 == {?bar} && {?foo}");
    EXPECT_EQ(data.value->to_string(), "false");

    // solve traversing
    d = dip::DIP();
    d.add_string(
        "snap int = 3\n"
        "foo.bar\n"
        "  crackle bool = false\n"
    );
    env = d.parse();
    dip::Path current("foo.bar");

    // solve absolute path
    solver = dip::LogicalSolver(env, current);
    data = solver.eval("{?foo.bar.crackle}");
    EXPECT_EQ(data.value->to_string(), "false");

    // solve relative path
    data = solver.eval("{..snap} == 3");
    EXPECT_EQ(data.value->to_string(), "true");
}

TEST(SolverLogical, ArrayAlgebra) {

    // solve expression
    dip::DIP d;
    d.add_string("foo int[3] = [-1,2,3]");
    dip::Environment env = d.parse();

    dip::LogicalSolver solver(env);
    dip::ValueNodeData data = solver.eval("{?foo} < 0");
    EXPECT_EQ(data.value->to_string(), "[true, false, false]");
}
