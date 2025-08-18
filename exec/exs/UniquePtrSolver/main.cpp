#import "main.h"

#include <memory>

int main() {

  exs::OperatorList<LogicalAtom> operators;
  operators.append(exs::AND_OPERATOR, std::make_shared<exs::OperatorAnd<LogicalAtom>>());

  exs::StepList steps;
  steps.append(exs::BINARY_OPERATION, {exs::AND_OPERATOR});

  exs::Solver<LogicalAtom> solver(operators, steps);
  LogicalAtom atom = solver.solve("true && false");
  atom.print();

  /*
   * Equivalent c++ notation:
   * true && false = false
   */
}
