#include "snt/exs.h"

#include <memory>

using namespace snt::exs;

int main() {

  // modifying default operator symbols
  OperatorList operators;
  operators.append(NOT_OPERATOR, std::make_shared<OperatorNot>("N"));
  operators.append(AND_OPERATOR, std::make_shared<OperatorAnd>("A"));
  operators.append(OR_OPERATOR, std::make_shared<OperatorOr>("O"));

  // changing default operation steps
  StepList steps {};
  steps.append(BINARY_OPERATION, {OR_OPERATOR});
  steps.append(BINARY_OPERATION, {AND_OPERATOR});
  steps.append(UNARY_OPERATION, {NOT_OPERATOR});

  Solver<Atom> solver(operators, steps);
  Atom atom = solver.solve("N false A false O true");
  atom.print();

  /*
   * Equivalent c++ notation:
   * !(false && (false || true)) == true
   */
}
