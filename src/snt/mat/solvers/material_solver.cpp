#include "material_solver.h"

namespace snt::mat {

  MaterialSolver::MaterialSolver() {

    exs::OperatorList<Substance> operators;
    operators.append(exs::PARENTHESES_OPERATOR, std::make_shared<exs::OperatorParentheses<Substance>>());
    operators.append(exs::MULTIPLY_OPERATOR, std::make_shared<exs::OperatorMultiply<Substance>>());
    operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd<Substance>>());

    exs::StepList steps;
    steps.append(exs::GROUP_OPERATION,  {exs::PARENTHESES_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::MULTIPLY_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::ADD_OPERATOR});
    
    solver = std::make_unique<exs::Solver<Substance>>(operators, steps);
  }
  
}
