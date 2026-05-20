#include <snt/dip/solvers/numerical_solver.h>
#include <snt/dip/solvers/numerical_atom.h>

namespace snt::dip {


  NumericalSolver::NumericalSolver(Environment& env, const std::string& units) {

    NumericalSettings settings = {{}, &env, units};

    exs::OperatorList operators;
    operators.append(
        exs::SINUS_OPERATOR,
        std::make_shared<exs::OperatorSinus>(exs::OperatorGroupSybols("sin", "( ", " )", ", ")));
    operators.append(
        exs::COSINUS_OPERATOR,
        std::make_shared<exs::OperatorCosinus>(exs::OperatorGroupSybols("cos", "( ", " )", ", ")));
    operators.append(
        exs::TANGENS_OPERATOR,
        std::make_shared<exs::OperatorTangens>(exs::OperatorGroupSybols("tan", "( ", " )", ", ")));
    operators.append(
        exs::CUBIC_ROOT_OPERATOR,
        std::make_shared<exs::OperatorCubicRoot>(exs::OperatorGroupSybols("cbrt", "( ", " )", ", ")));
    operators.append(
        exs::SQUARE_ROOT_OPERATOR,
        std::make_shared<exs::OperatorSquareRoot>(exs::OperatorGroupSybols("sqrt", "( ", " )", ", ")));
    // operators.append(
    //     exs::POWER_BASE_OPERATOR,
    //     std::make_shared<exs::OperatorPowerBase>("powb( ", " )"));
    // operators.append(
    //     exs::LOGARITHM_BASE_OPERATOR,
    //     std::make_shared<exs::OperatorLogarithmBase>("logb( ", " )"));
    operators.append(
        exs::LOGARITHM_10_OPERATOR,
        std::make_shared<exs::OperatorLogarithm10>(exs::OperatorGroupSybols("log10", "( ", " )", ", ")));
    operators.append(
        exs::LOGARITHM_OPERATOR,
        std::make_shared<exs::OperatorLogarithm>(exs::OperatorGroupSybols("log", "( ", " )", ", ")));
    operators.append(
        exs::EXPONENT_OPERATOR,
        std::make_shared<exs::OperatorExponent>(exs::OperatorGroupSybols("exp", "( ", " )", ", ")));
    operators.append(
        exs::PARENTHESES_OPERATOR,
        std::make_shared<exs::OperatorParentheses>(exs::OperatorGroupSybols("", "( ", " )", ", ")));
    operators.append(exs::POWER_OPERATOR,
                     std::make_shared<exs::OperatorPower>(" ** "));
    operators.append(exs::MULTIPLY_OPERATOR,
                     std::make_shared<exs::OperatorMultiply>(" * "));
    operators.append(exs::DIVIDE_OPERATOR,
                     std::make_shared<exs::OperatorDivide>(" / "));
    operators.append(exs::ADD_OPERATOR,
                     std::make_shared<exs::OperatorAdd>(" +"));
    operators.append(exs::SUBTRACT_OPERATOR,
                     std::make_shared<exs::OperatorSubtract>(" -"));

    exs::StepList steps;
    steps.append(exs::GROUP_OPERATION, {exs::EXPONENT_OPERATOR, exs::LOGARITHM_OPERATOR,
                                        exs::LOGARITHM_10_OPERATOR,
                                        // exs::LOGARITHM_BASE_OPERATOR, exs::POWER_BASE_OPERATOR,
                                        exs::SQUARE_ROOT_OPERATOR,
                                        exs::CUBIC_ROOT_OPERATOR,
                                        exs::SINUS_OPERATOR, exs::COSINUS_OPERATOR, exs::TANGENS_OPERATOR});
    steps.append(exs::GROUP_OPERATION, {exs::PARENTHESES_OPERATOR});
    steps.append(exs::UNARY_OPERATION, {exs::ADD_OPERATOR, exs::SUBTRACT_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::POWER_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::MULTIPLY_OPERATOR, exs::DIVIDE_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::ADD_OPERATOR, exs::SUBTRACT_OPERATOR});

    solver = std::make_unique<exs::Solver<NumericalAtom, NumericalSettings>>(operators, steps, settings);
  }

  NumericalAtom NumericalSolver::eval(const std::string& expression) {
    if (expression.empty())
      throw std::runtime_error("Numerical expression cannot be empty");
    NumericalAtom ua = solver->solve(expression);
    return ua;
  }

} // namespace snt::dip
