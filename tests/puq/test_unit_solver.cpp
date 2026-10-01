#include "pch_tests.h"

#include <snt/puq/solver/unit_atom.h>
#include <snt/puq/solver/unit_solver.h>

using namespace snt;

TEST(UnitSolver, Initialization) {

    puq::UnitSolver solver;
}

TEST(UnitSolver, Solve) {

    puq::UnitSolver solver;

    puq::UnitAtom atom = solver.eval(""); // empty string
    EXPECT_EQ(atom.value.to_string(), "1");

    atom = solver.eval("3*(2.0e1/5.0)"); // only numbers
    EXPECT_EQ(atom.value.to_string(), "12");

    atom = solver.eval("2.4*km/s");
    EXPECT_EQ(atom.value.to_string(), "2.4*km*s-1"); // simple arithmetics

    atom = solver.eval("3.2e4*kg*m*m/s/s"); // exponent reduction
    EXPECT_EQ(atom.value.to_string(), "3.2e4*kg*m2*s-2");

    atom = solver.eval("kg/(m*s)"); // simple parentheses
    EXPECT_EQ(atom.value.to_string(), "kg*m-1*s-1");

    atom = solver.eval("kg/(m*s)2*C"); // parentheses with eponents
    EXPECT_EQ(atom.value.to_string(), "kg*m-2*s-2*C");

    atom = solver.eval("(kg*s/(m2*K))2"); // nested parentheses with an exponent
    EXPECT_EQ(atom.value.to_string(), "kg2*s2*m-4*K-2");

    atom = solver.eval("kg*<v>2"); // quantities
    EXPECT_EQ(atom.value.to_string(), "kg*<v>2");
}

TEST(UnitSolver, SolveFractions) {

    puq::UnitSolver solver;
    puq::UnitAtom atom = solver.eval("km-2:3"); // units with fractions
    EXPECT_EQ(atom.value.to_string(), "km-2:3");

    atom = solver.eval("m1:2*m-3:5"); // reduction of fractions
    EXPECT_EQ(atom.value.to_string(), "m-1:10");

    atom = solver.eval("kg/(m*s)-1:2*C"); // parentheses with fractions
    EXPECT_EQ(atom.value.to_string(), "kg*m1:2*s1:2*C");

    atom = solver.eval("(kg3*s4/(m2*K))1:2"); // nested parentheses with an exponent
    EXPECT_EQ(atom.value.to_string(), "kg3:2*s2*m-1*K-1:2");
}

TEST(UnitSolver, SolveArrays) {

    puq::UnitSolver solver;
    puq::UnitAtom atom = solver.eval("[20, 40.5, 6.8e1]"); // numerical array only
    EXPECT_EQ(atom.value.to_string(), "[20, 40.5, 68]");

    atom = solver.eval("[20, 40.5, 6.8e1]*kg/s"); // numerical array with units
    EXPECT_EQ(atom.value.to_string(), "[20, 40.5, 68]*kg*s-1");

    atom = solver.eval("[20, 40.5, 6.8e1]*2"); // multiplied by a scalar
    EXPECT_EQ(atom.value.to_string(), "[40, 81, 136]");

    atom = solver.eval("[20, 40.5]*[2,3]"); // multiplication of vectors
    EXPECT_EQ(atom.value.to_string(), "[40, 121.5]");
}

TEST(UnitSolver, OptionalCompositionPreservesCustomOperators) {
    exs::CompositionGraph graph;
    auto result = puq::UnitSolver::solver.eval("[20,40.5]*kg/(m*s)2", &graph);
    EXPECT_EQ(result.value.to_string(), "[20, 40.5]*kg*m-2*s-2");
    ASSERT_LT(graph.root, graph.nodes.size());
    EXPECT_EQ(graph.nodes.at(graph.root).text, "div");

    exs::CompositionGraph short_graph;
    auto short_array = puq::UnitSolver::solver.eval("[2,3]", &short_graph);
    EXPECT_EQ(short_array.value.to_string(), "[2, 3]");
    EXPECT_EQ(short_graph.nodes.at(short_graph.root).children.size(), 2);

    auto before_inspection = puq::UnitSolver::solver.eval("(kg*s)2/(m)3");
    const auto structure =
        exs::build_composition_graph("(kg*s)2/(m)3", puq::UnitSolver::solver.operators, puq::UnitSolver::solver.steps);
    EXPECT_EQ(structure.nodes.at(structure.root).text, "div");
    auto after_inspection = puq::UnitSolver::solver.eval("(kg*s)2/(m)3");
    EXPECT_EQ(after_inspection.value.to_string(), before_inspection.value.to_string());
}

TEST(UnitSolver, SolveErrors) {

    puq::UnitSolver solver;
    puq::UnitAtom atom = solver.eval("{#m_p}");

    puq::Measurement uv = atom.value.convert(puq::Format::Base::MKS);
    EXPECT_EQ(uv.to_string(), "1.67262192595(52)e-27*kg");

    puq::Dimensions dim = atom.value.baseunits.dimensions();
    EXPECT_EQ(dim.to_string(), "1.67262192595(52)e-24*g");
}
