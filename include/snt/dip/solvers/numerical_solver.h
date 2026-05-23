#ifndef DIP_NUMERICAL_SOLVER_H
#define DIP_NUMERICAL_SOLVER_H

#include <snt/dip/environment.h>
#include <snt/dip/solvers/numerical_atom.h>

namespace snt::dip {

    class NumericalSolver {
      public:
        std::unique_ptr<exs::Solver<NumericalAtom, NumericalSettings>> solver;
        NumericalSolver(Environment& env);
        ValueNodeData eval(const std::string& expression, const std::string& units = "");
    };

} // namespace snt::dip

#endif // DIP_NUMERICAL_SOLVER_H
