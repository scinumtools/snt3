#ifndef DIP_LOGICAL_SOLVER_H
#define DIP_LOGICAL_SOLVER_H

#include <snt/dip/environment.h>
#include <snt/dip/solvers/logical_atom.h>

namespace snt::dip {

    /**
     * @class LogicalSolver
     * @brief Solves logical expressions of DIP nodes
     */
    class LogicalSolver {
      public:
        std::unique_ptr<exs::Solver<LogicalAtom, LogicalSettings>> solver; ///< Instance of an EXS expression solver

        LogicalSolver(Environment& env);

        /**
         * @brief Solve logical DIP expression
         *
         * @param expression Logical expression string
         * @returns Data of a value node
         */
        ValueNodeData eval(const std::string& expression);
    };

} // namespace snt::dip

#endif // DIP_LOGICAL_SOLVER_H
