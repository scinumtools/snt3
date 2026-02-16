#include <snt/puq/calc/calculator.h>

#include <memory>
#include <regex>

namespace snt::puq {

  Calculator::Calculator() {

    exs::OperatorList operators;
    operators.append(exs::PARENTHESES_OPERATOR, std::make_shared<exs::OperatorGroup<1>>("par", exs::OperatorGroupSybols("", "( ", " )", " , "), exs::PARENTHESES_OPERATOR));
    operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd>(" + "));
    operators.append(exs::SUBTRACT_OPERATOR, std::make_shared<exs::OperatorSubtract>(" - "));
    operators.append(exs::MULTIPLY_OPERATOR, std::make_shared<exs::OperatorMultiply>(" * "));
    operators.append(exs::DIVIDE_OPERATOR, std::make_shared<exs::OperatorDivide>(" / "));

    exs::StepList steps;
    steps.append(exs::GROUP_OPERATION, {exs::PARENTHESES_OPERATOR});
    steps.append(exs::UNARY_OPERATION, {exs::ADD_OPERATOR, exs::SUBTRACT_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::MULTIPLY_OPERATOR, exs::DIVIDE_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::ADD_OPERATOR, exs::SUBTRACT_OPERATOR});

    solver = std::make_unique<exs::Solver<CalculatorAtom>>(operators, steps);
  }

  CalculatorAtom Calculator::solve(const std::string& expression) {
    if constexpr (Config::debug_calculator) {
      std::clog << "CALC:  Solving: " << expression << std::endl;
    }
    CalculatorAtom ca = solver->solve((expression == "") ? "1" : expression);
    if constexpr (Config::debug_calculator) {
      std::clog << "CALC:  Result:  " << ca.value.to_string() << std::endl;
    }
    return ca;
  }

} // namespace snt::puq
