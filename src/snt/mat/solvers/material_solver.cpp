#include <snt/mat/solvers/material_solver.h>

namespace snt::mat {

    // exs::Solver<MaterialAtom> MaterialSolver::solver = [] {
    //     exs::OperatorList operators;
    //     operators.append(exs::PARENTHESES_OPERATOR, std::make_shared<exs::OperatorParentheses>());
    //     operators.append(exs::MULTIPLY_OPERATOR, std::make_shared<exs::OperatorMultiply>());
    //     operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd>());
    //
    //     exs::StepList steps;
    //     steps.append(exs::GROUP_OPERATION, {exs::PARENTHESES_OPERATOR});
    //     steps.append(exs::BINARY_OPERATION, {exs::MULTIPLY_OPERATOR});
    //     steps.append(exs::BINARY_OPERATION, {exs::ADD_OPERATOR});
    //
    //     return exs::Solver<MaterialAtom>(operators, steps);
    // }();

    MaterialSolver::MaterialSolver() {}

} // namespace snt::mat
