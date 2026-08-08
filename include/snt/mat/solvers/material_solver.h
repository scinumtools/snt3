#ifndef MAT_MATERIAL_SOLVER_H
#define MAT_MATERIAL_SOLVER_H

#include <snt/exs/solver.h>
#include <snt/mat/substance.h>

namespace snt::mat {

    /**
     * This class solves material expressions
     */
    class MaterialSolver {
      public:
        // static exs::Solver<MaterialAtom> solver; ///< Pointer to the EXS solver object

        /**
         * Constructor of this class
         */
        MaterialSolver();
    };

} // namespace snt::mat

#endif // MAT_MATERIAL_SOLVER_H
