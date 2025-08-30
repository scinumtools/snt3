#import "main.h"

#import "exs/comparison/less.h"
#import "exs/solver.h"
#import "exs/step_list.h"

#include <memory>

int main() {

  // modifying default operator symbols
  exs::OperatorList<CustomAtom> operators;
  operators.append(exs::LESS_OPERATOR, std::make_shared<exs::OperatorLess<CustomAtom>>());
  operators.append(LENGTH_OPERATOR, std::make_shared<OperatorLength>());

  // changing default operation steps
  exs::StepList steps;
  steps.append(exs::GROUP_OPERATION, {LENGTH_OPERATOR});
  steps.append(exs::BINARY_OPERATION, {exs::LESS_OPERATOR});

  exs::Solver<CustomAtom> solver(operators, steps);
  CustomAtom atom = solver.solve("apple < len(hospital)");
  atom.print();

  /*
   * Equivalent c++ notation:
   * std::string("apple").size() < std::string("hospital").size() == true
   */
}
