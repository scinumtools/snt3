#include "pch_tests.h"

#include <snt/exs/atom.h>
#include <snt/exs/solver.h>

using namespace snt;

TEST(Composition, PrecedenceAndResult) {
    exs::Solver<exs::Atom> solver;
    exs::CompositionGraph graph;
    auto value = solver.eval("1 + 2 * 3", &graph);
    EXPECT_EQ(value.to_string(), "7");
    const auto& root = graph.nodes.at(graph.root);
    EXPECT_EQ(root.kind, exs::CompositionKind::Operator);
    EXPECT_EQ(root.text, "add");
    ASSERT_EQ(root.children.size(), 2);
    EXPECT_EQ(graph.nodes.at(root.children[0]).text, "1");
    const auto& product = graph.nodes.at(root.children[1]);
    EXPECT_EQ(product.text, "mul");
    ASSERT_EQ(product.children.size(), 2);
    EXPECT_EQ(graph.nodes.at(product.children[0]).text, "2");
    EXPECT_EQ(graph.nodes.at(product.children[1]).text, "3");
    EXPECT_EQ(solver.eval("1 + 2 * 3").to_string(), value.to_string());
}

TEST(Composition, GroupsUnaryAndCondition) {
    exs::Solver<exs::Atom> solver;
    exs::CompositionGraph grouped;
    auto negative = solver.eval("-(1+2)", &grouped);
    EXPECT_EQ(negative.to_string(), "-3");
    const auto& negate = grouped.nodes.at(grouped.root);
    EXPECT_EQ(negate.text, "sub");
    ASSERT_EQ(negate.children.size(), 1);
    const auto& par = grouped.nodes.at(negate.children[0]);
    EXPECT_EQ(par.kind, exs::CompositionKind::Group);
    EXPECT_EQ(par.text, "par");
    EXPECT_EQ(grouped.nodes.at(par.children.at(0)).text, "add");

    exs::CompositionGraph conditional;
    auto selected = solver.eval("true ? 2 : 3", &conditional);
    EXPECT_EQ(selected.to_string(), "2");
    const auto& condition = conditional.nodes.at(conditional.root);
    EXPECT_EQ(condition.text, "cond");
    ASSERT_EQ(condition.children.size(), 3);
    EXPECT_EQ(conditional.nodes.at(condition.children[0]).text, "true");
    EXPECT_EQ(conditional.nodes.at(condition.children[1]).text, "2");
    EXPECT_EQ(conditional.nodes.at(condition.children[2]).text, "3");
}

TEST(Composition, GroupArgumentsAndIndependentCalls) {
    exs::Solver<exs::Atom> solver;
    exs::CompositionGraph graph;
    auto value = solver.eval("logb(8,2)", &graph);
    EXPECT_EQ(value.to_string(), "3");
    const auto& root = graph.nodes.at(graph.root);
    EXPECT_EQ(root.kind, exs::CompositionKind::Group);
    EXPECT_EQ(root.text, "logb");
    ASSERT_EQ(root.children.size(), 2);
    EXPECT_EQ(graph.nodes.at(root.children[0]).text, "8");
    EXPECT_EQ(graph.nodes.at(root.children[1]).text, "2");

    exs::CompositionGraph separate;
    auto other = solver.eval("4", &separate);
    EXPECT_EQ(other.to_string(), "4");
    ASSERT_EQ(separate.nodes.size(), 1);
    EXPECT_EQ(separate.nodes.at(separate.root).text, "4");
}

TEST(Composition, CustomReductionOrder) {
    exs::StepList steps;
    steps.append(exs::BINARY_OPERATION, {exs::ADD_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::MULTIPLY_OPERATOR});
    exs::Solver<exs::Atom> solver(steps);

    exs::CompositionGraph graph;
    auto result = solver.eval("1 + 2 * 3", &graph);
    EXPECT_EQ(result.to_string(), "9");
    const auto& product = graph.nodes.at(graph.root);
    EXPECT_EQ(product.text, "mul");
    EXPECT_EQ(graph.nodes.at(product.children.at(0)).text, "add");
    EXPECT_EQ(graph.nodes.at(product.children.at(1)).text, "3");
}

TEST(Composition, StructuralPassPreservesDomainOperands) {
    exs::Solver<exs::Atom> solver;
    const auto graph =
        exs::build_composition_graph("sample.signal + calibration.offset * 2", solver.operators, solver.steps);
    const auto& sum = graph.nodes.at(graph.root);
    EXPECT_EQ(sum.text, "add");
    EXPECT_EQ(graph.nodes.at(sum.children.at(0)).text, "sample.signal");
    const auto& product = graph.nodes.at(sum.children.at(1));
    EXPECT_EQ(product.text, "mul");
    EXPECT_EQ(graph.nodes.at(product.children.at(0)).text, "calibration.offset");
    EXPECT_EQ(graph.nodes.at(product.children.at(1)).text, "2");
}

TEST(Composition, UsesOrdinaryEvaluationForSignFolding) {
    exs::Solver<exs::Atom> solver;
    for (const auto& expression : {"--2", "-+2", "+-2", "1--2", "1+-2", "2*-3"}) {
        auto plain = solver.eval(expression);
        exs::CompositionGraph traced;
        auto value = solver.eval(expression, &traced);
        EXPECT_EQ(value.to_string(), plain.to_string()) << expression;
        ASSERT_LT(traced.root, traced.nodes.size()) << expression;
    }
    exs::CompositionGraph graph;
    solver.eval("--2", &graph);
    const auto& outer = graph.nodes.at(graph.root);
    EXPECT_EQ(outer.text, "sub");
    const auto& inner = graph.nodes.at(outer.children.at(0));
    EXPECT_EQ(inner.text, "sub");
    EXPECT_EQ(graph.nodes.at(inner.children.at(0)).text, "2");
}

TEST(Composition, StandaloneAndEvaluatedStructureAgree) {
    exs::Solver<exs::Atom> solver;
    for (const auto& expression :
         {"1 + 2 * 3",
          "-(1+2)",
          "logb(8,2)",
          "true ? 2 : 3",
          "!false && true",
          "--2",
          "1--2",
          "1+-2",
          "1-+2",
          "1++2",
          "1---2"}) {
        const auto structural = exs::build_composition_graph(expression, solver.operators, solver.steps);
        exs::CompositionGraph evaluated;
        solver.eval(expression, &evaluated);
        const auto compare = [&](const auto& self, std::size_t lhs, std::size_t rhs) -> void {
            const auto& a = structural.nodes.at(lhs);
            const auto& b = evaluated.nodes.at(rhs);
            EXPECT_EQ(a.kind, b.kind) << expression;
            EXPECT_EQ(a.text, b.text) << expression;
            EXPECT_EQ(a.operator_type, b.operator_type) << expression;
            EXPECT_EQ(a.operation, b.operation) << expression;
            ASSERT_EQ(a.children.size(), b.children.size()) << expression;
            for (std::size_t i = 0; i < a.children.size(); ++i)
                self(self, a.children[i], b.children[i]);
        };
        compare(compare, structural.root, evaluated.root);
    }
}
