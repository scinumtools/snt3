#ifndef DIP_NUMERICAL_SOLVER_H
#define DIP_NUMERICAL_SOLVER_H

#include <snt/dip/environment.h>
#include <snt/dip/solvers/numerical_atom.h>

namespace snt::dip {

    /**
     * Solves numerical expressions of DIP nodes
     */
    class NumericalSolver {
      public:
        static exs::Solver<NumericalAtom, NumericalSettings> solver; ///< Instance of an EXS expression solver

        /**
         * Numerical solver constructor
         *
         * @param env Environment class object
         * @param current Current path in the node hierarchy
         */
        NumericalSolver(Environment& env, Path current = Path(""));

        /**
         * Solve numerical DIP expression
         *
         * @param expression Numerical expression string
         * @param units Target units of a solved value node
         * @returns Data of a value node
         */
        ValueNodeData eval(const std::string& expression, const std::string& units = "");
    };

} // namespace snt::dip

#endif // DIP_NUMERICAL_SOLVER_H
