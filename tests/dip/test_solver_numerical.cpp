#include "pch_tests.h"

#include <snt/dip/dip.h>
#include <snt/dip/environment.h>
#include <snt/dip/solvers/numerical_solver.h>

using namespace snt;

TEST(SolverNumerical, AddSubtract) {

    dip::Environment env;
    dip::NumericalSolver solver(env);
    dip::ValueNodeData data = solver.eval("1.23 + 4.56");
    EXPECT_EQ(data.value->to_string(), "5.79");

    data = solver.eval("4.56 - 1.23");
    EXPECT_EQ(data.value->to_string(), "3.33");

    data = solver.eval("1.23 + 4.56 - 7.89");
    EXPECT_EQ(data.value->to_string(), "-2.1");

    data = solver.eval("+1.23");
    EXPECT_EQ(data.value->to_string(), "1.23");

    data = solver.eval("-1.23");
    EXPECT_EQ(data.value->to_string(), "-1.23");

    data = solver.eval("1.23 + -2.34");
    EXPECT_EQ(data.value->to_string(), "-1.11");
}

TEST(SolverNumerical, MultiplyDivide) {

    dip::Environment env;
    dip::NumericalSolver solver(env);
    dip::ValueNodeData data = solver.eval("1.23 * 4.56");
    EXPECT_EQ(data.value->to_string(), "5.609");

    data = solver.eval("4.56 / 1.23");
    EXPECT_EQ(data.value->to_string(), "3.707");

    data = solver.eval("1.23 * 4.56 / 7.89");
    EXPECT_EQ(data.value->to_string(), "0.7109");
}

TEST(SolverNumerical, GroupOperators) {

    dip::Environment env;
    dip::NumericalSolver solver(env);
    dip::ValueNodeData data = solver.eval("1.23 ** 4.56");
    EXPECT_EQ(data.value->to_string(), "2.57");

    data = solver.eval("exp( 1.23 )");
    EXPECT_EQ(data.value->to_string(), "3.421");

    data = solver.eval("log( 19.0 )");
    EXPECT_EQ(data.value->to_string(), "2.944");

    data = solver.eval("log10( 19.0 )");
    EXPECT_EQ(data.value->to_string(), "1.279");

    data = solver.eval("sqrt( 9.0 )");
    EXPECT_EQ(data.value->to_string(), "3");

    data = solver.eval("cbrt( 8.0 )");
    EXPECT_EQ(data.value->to_string(), "2");

    data = solver.eval("sin( 8.0 )");
    EXPECT_EQ(data.value->to_string(), "0.9894");

    data = solver.eval("cos( 8.0 )");
    EXPECT_EQ(data.value->to_string(), "-0.1455");

    data = solver.eval("tan( 8.0 )");
    EXPECT_EQ(data.value->to_string(), "-6.8");

    data = solver.eval("2.0 * ( 8.0 - 4.0 )");
    EXPECT_EQ(data.value->to_string(), "8");

    data = solver.eval("cos( 2.0 * ( sqrt( 16.0 ) - 2.0 ) )");
    EXPECT_EQ(data.value->to_string(), "-0.6536");
}

TEST(SolverNumerical, Injections) {

    dip::DIP d;
    d.add_string("foo float = 4.52");
    d.add_string("bar int = 3");
    dip::Environment env = d.parse();

    dip::NumericalSolver solver(env);
    dip::ValueNodeData data = solver.eval("sin( {?foo} ) - 2 * {?bar}");
    EXPECT_EQ(data.value->to_string(), "-6.982");
}
