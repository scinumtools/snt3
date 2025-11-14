#include "substance_solver.h"

namespace snt::mat {

  SubstanceSolver::SubstanceSolver() {

    exs::OperatorList<Element> operators;
    operators.append(exs::PARENTHESES_OPERATOR, std::make_shared<exs::OperatorParentheses<Element>>());
    operators.append(exs::MULTIPLY_OPERATOR, std::make_shared<exs::OperatorMultiply<Element>>());
    operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd<Element>>());

    exs::StepList steps;
    steps.append(exs::GROUP_OPERATION,  {exs::PARENTHESES_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::MULTIPLY_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::ADD_OPERATOR});
    
    solver = std::make_unique<exs::Solver<Element>>(operators, steps);
  }
  
}
