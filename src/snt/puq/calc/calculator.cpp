#include <memory>
#include <regex>
#include <snt/puq/calc/calculator.h>

namespace snt::puq {

    exs::Solver<CalculatorAtom> Calculator::solver = [] {
        exs::OperatorList operators;
        operators.append(
            exs::PARENTHESES_OPERATOR,
            std::make_shared<exs::OperatorGroup<1>>(
                "par", exs::OperatorGroupSybols{"", "( ", " )", " , "}, exs::PARENTHESES_OPERATOR
            )
        );
        operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd>(" + "));
        operators.append(exs::SUBTRACT_OPERATOR, std::make_shared<exs::OperatorSubtract>(" - "));
        operators.append(exs::MULTIPLY_OPERATOR, std::make_shared<exs::OperatorMultiply>(" * "));
        operators.append(exs::DIVIDE_OPERATOR, std::make_shared<exs::OperatorDivide>(" / "));

        exs::StepList steps;
        steps.append(exs::GROUP_OPERATION, {exs::PARENTHESES_OPERATOR});
        steps.append(exs::UNARY_OPERATION, {exs::ADD_OPERATOR, exs::SUBTRACT_OPERATOR});
        steps.append(exs::BINARY_OPERATION, {exs::MULTIPLY_OPERATOR, exs::DIVIDE_OPERATOR});
        steps.append(exs::BINARY_OPERATION, {exs::ADD_OPERATOR, exs::SUBTRACT_OPERATOR});

        return exs::Solver<CalculatorAtom>(operators, steps);
    }();

    Calculator::Calculator() {}

    CalculatorAtom Calculator::eval(const std::string& expression) {
        if constexpr (Config::debug_calculator) {
            std::clog << "CALC:  Solving: " << expression << '\n';
        }
        CalculatorAtom ca = solver.eval((expression == "") ? "1" : expression);
        if constexpr (Config::debug_calculator) {
            std::clog << "CALC:  Result:  " << ca.value.to_string() << '\n';
        }
        return ca;
    }

} // namespace snt::puq
