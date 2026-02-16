#include <snt/puq/solver/unit_solver.h>

#include "operator_array.h"
#include "operator_parentheses.h"

#include <memory>
#include <regex>

namespace snt::puq {

  UnitSolver::UnitSolver() {

    exs::OperatorList operators;
    operators.append(exs::PARENTHESES_OPERATOR, std::make_shared<OperatorParentheses>());
#if defined(MAGNITUDE_VALUES)
    operators.append(ARRAY_OPERATOR, std::make_shared<OperatorArray>());
#endif
    operators.append(exs::MULTIPLY_OPERATOR, std::make_shared<exs::OperatorMultiply>(std::string(Symbols::multiply)));
    operators.append(exs::DIVIDE_OPERATOR, std::make_shared<exs::OperatorDivide>(std::string(Symbols::divide)));

    exs::StepList steps;
#if defined(MAGNITUDE_VALUES)
    steps.append(exs::GROUP_OPERATION, {exs::PARENTHESES_OPERATOR, ARRAY_OPERATOR});
#else
    steps.append(exs::GROUP_OPERATION, {exs::PARENTHESES_OPERATOR});
#endif
    steps.append(exs::BINARY_OPERATION, {exs::MULTIPLY_OPERATOR, exs::DIVIDE_OPERATOR});

    solver = std::make_unique<exs::Solver<UnitAtom>>(operators, steps);
  }

  UnitAtom UnitSolver::solve(const std::string& expression) {
    if constexpr (Config::debug_unit_solver) {
      std::clog << "UNIT:  Solving: " << expression << std::endl;
    }
    UnitAtom ua = solver->solve((expression == "") ? "1" : expression);
    if constexpr (Config::debug_unit_solver) {
      std::clog << "UNIT:  Result:  " << ua.value.to_string() << std::endl;
    }
    return ua;
  }

} // namespace snt::puq
