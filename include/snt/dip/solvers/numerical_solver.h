#ifndef DIP_NUMERICAL_SOLVER_H
#define DIP_NUMERICAL_SOLVER_H

#include <snt/dip/environment.h>
#include <snt/dip/solvers/numerical_atom.h>

namespace snt::dip {

    /**
     * @class NumericalSolver
     * @brief Solves numerical expressions of DIP nodes
     */
    class NumericalSolver {
      public:
        std::unique_ptr<exs::Solver<NumericalAtom, NumericalSettings>> solver; ///< Instance of an EXS expression solver

        NumericalSolver(Environment& env);

        /**
         * @brief Solve numerical DIP expression
         *
         * @param expression Numerical expression string
         * @param units Target units of a solved value node
         * @returns Data of a value node
         */
        ValueNodeData eval(const std::string& expression, const std::string& units = "");
    };

} // namespace snt::dip

#endif // DIP_NUMERICAL_SOLVER_H
