#ifndef DIP_LOGICAL_SOLVER_H
#define DIP_LOGICAL_SOLVER_H

#include <snt/dip/environment.h>
#include <snt/dip/solvers/logical_atom.h>

namespace snt::dip {

    class LogicalSolver {
      public:
        std::unique_ptr<exs::Solver<LogicalAtom, LogicalSettings>> solver;
        LogicalSolver(Environment& env);
        LogicalAtom eval(const std::string& expression);
    };

} // namespace snt::dip

#endif // DIP_LOGICAL_SOLVER_H
