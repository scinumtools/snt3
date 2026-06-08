#ifndef PUQ_UNIT_SOLVER_H
#define PUQ_UNIT_SOLVER_H

#include <snt/exs/solver.h>
#include <snt/puq/solver/unit_atom.h>
#include <unordered_map>

namespace snt::puq {

    /**
     * @class UnitSolver
     * @brief Calcuates unit atoms from unit expressions
     *
     * In order to speed up calculations of units this class is caching input unit expression strings and
     * stores already calculated unit atoms in a cache map. This should avoid repeated calculations
     * of the same inputs.
     */
    class UnitSolver {
      public:
        static exs::Solver<UnitAtom> solver;                    ///< static instance of an unit solver
        static std::unordered_map<std::string, UnitAtom> cache; ///< list of previously calculated atoms

        /**
         * @brief Default constructor
         */
        UnitSolver() = default;

        /**
         * @brief Solve unit expression
         *
         * @param expression Unit expression that will be solved
         * @return Solved unit atom
         */
        UnitAtom eval(const std::string& expression);
    };

} // namespace snt::puq

#endif // PUQ_UNIT_SOLVER_H
