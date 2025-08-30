#import "main.h"

#import "exs/math/add.h"
#import "exs/math/subtract.h"
#import "exs/operator_list.h"
#import "exs/solver.h"
#import "exs/step_list.h"

#include <memory>

int main() {

  // modifying default operator symbols
  exs::OperatorList<CustomAtom> operators;
  operators.append(exs::ADD_OPERATOR, std::make_shared<exs::OperatorAdd<CustomAtom>>());
  operators.append(exs::SUBTRACT_OPERATOR, std::make_shared<exs::OperatorSubtract<CustomAtom>>());
  operators.append(ARRAY_OPERATOR, std::make_shared<OperatorArray>());

  // changing default operation steps
  exs::StepList steps;
  steps.append(exs::GROUP_OPERATION, {ARRAY_OPERATOR});
  steps.append(exs::BINARY_OPERATION, {exs::ADD_OPERATOR, exs::SUBTRACT_OPERATOR});

  exs::Solver<CustomAtom> solver(operators, steps);
  CustomAtom atom = solver.solve("2 + 3");
  atom.print();

  atom = solver.solve("[1,2,3] + [4,5,6]");
  atom.print();

  /*
   * Equivalent c++ notation:
   * std::vector<double> v1 = {1,2,3};
   * std::vector<double> v2 = {4,5,6};
   * for (size_t i=0; i<v1.size(); i++)
   *    v1[i] = v1[i] + v2[i];
   * # v1 == {5,6,9}
   */
}
