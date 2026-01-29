#import "main.h"

#include <memory>

int main() {

  Settings settings({"?", 5, {1, 2, 3, 4, 5}});

  // modifying default operator symbols
  exs::OperatorList<Settings> operators;
  operators.append(SELECT_OPERATOR, std::make_shared<OperatorSelect>());
  operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd<Settings>>());
  operators.append(exs::SUBTRACT_OPERATOR, std::make_shared<exs::OperatorSubtract<Settings>>());

  // changing default operation steps
  exs::StepList steps {};
  steps.append(exs::GROUP_OPERATION, {SELECT_OPERATOR});
  steps.append(exs::BINARY_OPERATION, {exs::ADD_OPERATOR, exs::SUBTRACT_OPERATOR});

  exs::Solver<CustomAtom, Settings> solver(operators, steps, &settings);
  CustomAtom atom = solver.solve("2 + ? - {3}");
  atom.print();

  /*
   * Equivalent c++ notation:
   * 2 + 5 - 4 = 3
   *
   * here the ? is directly substituted by "5"
   * {3} operator selects third value "4" from the option list in the settings
   */
}
