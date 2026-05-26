#include "pch_tests.h"

#include <snt/dip/dip.h>
#include <snt/dip/solvers/template_solver.h>

using namespace snt;

TEST(SolverTemplate, BasicParsing) {

    dip::DIP d;
    d.add_string("foo float = 23 cm");
    d.add_string("bar str = \"baz\"");
    dip::Environment env = d.parse();

    dip::TemplateSolver solver(env);
    dip::ValueNodeData data = solver.eval("{{?foo}}, {{?bar}}");
    EXPECT_EQ(data.value->to_string(), "\"23 cm, baz\"");
}

TEST(SolverTemplate, NumberFormatting) {

    dip::DIP d;
    d.add_string("foo float = 1.23456e7 cm");
    dip::Environment env = d.parse();

    dip::TemplateSolver solver(env);
    dip::ValueNodeData data;

    data = solver.eval("foo: {{?foo}}");
    EXPECT_EQ(data.value->to_string(), "\"foo: 1.235e7 cm\"");

    // TODO: this needs to be changed, because of the inconsistencies
    //       in comparison to the Python. Probably c++20 format should be used

    // decimal precision
    data = solver.eval("foo: {{?foo}:.5g}");
    EXPECT_EQ(data.value->to_string(), "\"foo: 1.23456e7 cm\"");
    data = solver.eval("foo: {{?foo}:.5f}");
    EXPECT_EQ(data.value->to_string(), "\"foo: 12345600.00000 cm\"");
    data = solver.eval("foo: {{?foo}:.2e}");
    EXPECT_EQ(data.value->to_string(), "\"foo: 1.23e+07 cm\"");

    // padding with an empty character
    data = solver.eval("foo: {{?foo}:9g}");
    EXPECT_EQ(data.value->to_string(), "\"foo:   1.235e7 cm\"");
    data = solver.eval("foo: {{?foo}:11.1f}");
    EXPECT_EQ(data.value->to_string(), "\"foo:  12345600.0 cm\"");
    data = solver.eval("foo: {{?foo}:11e}");
    EXPECT_EQ(data.value->to_string(), "\"foo:  1.2346e+07 cm\"");

    // padding with zeros
    data = solver.eval("foo: {{?foo}:09g}");
    EXPECT_EQ(data.value->to_string(), "\"foo: 001.235e7 cm\"");
    data = solver.eval("foo: {{?foo}:011.1f}");
    EXPECT_EQ(data.value->to_string(), "\"foo: 012345600.0 cm\"");
    data = solver.eval("foo: {{?foo}:011e}");
    EXPECT_EQ(data.value->to_string(), "\"foo: 01.2346e+07 cm\"");
}

TEST(SolverTemplate, ArraySlicing) {

    dip::DIP d;
    d.add_string("foo int[3,3] = [[1,2,3],[4,5,6],[7,8,9]]");
    dip::Environment env = d.parse();

    dip::TemplateSolver solver(env);
    dip::ValueNodeData data;

    data = solver.eval("foo: {{?foo}}");
    EXPECT_EQ(data.value->to_string(), "\"foo: [[1, 2, 3], [4, 5, 6], [7, 8, 9]]\"");

    data = solver.eval("foo: {{?foo}[:,1]}");
    EXPECT_EQ(data.value->to_string(), "\"foo: [2, 5, 8]\"");
}

TEST(SolverTemplate, StringFormatting) {

    dip::DIP d;
    d.add_string("foo str = \"bar\"");
    dip::Environment env = d.parse();

    dip::TemplateSolver solver(env);
    dip::ValueNodeData data;

    data = solver.eval("foo: {{?foo}}");
    EXPECT_EQ(data.value->to_string(), "\"foo: bar\"");

    data = solver.eval("foo: {{?foo}:5s}");
    EXPECT_EQ(data.value->to_string(), "\"foo:   bar\"");
}
