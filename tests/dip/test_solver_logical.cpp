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
