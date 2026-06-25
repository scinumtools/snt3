#include "main.h"

#include "snt/exs/operator_list.h"
#include "snt/exs/operators/logical.h"
#include "snt/exs/solver.h"
#include "snt/exs/step_list.h"

#include <memory>

int main() {

    exs::OperatorList operators;
    operators.append(exs::AND_OPERATOR, std::make_shared<exs::OperatorAnd>());

    exs::StepList steps{};
    steps.append(exs::BINARY_OPERATION, {exs::AND_OPERATOR});

    exs::Solver<LogicalAtom> solver(operators, steps);
    LogicalAtom atom = solver.solve("true && false");
    atom.print();

    /*
     * Equivalent c++ notation:
     * true && false = false
     */
}
