#ifndef MAT_MATERIAL_SOLVER_H
#define MAT_MATERIAL_SOLVER_H

#include "../../exs.h"
#include "../substance.h"

namespace snt::mat {

  /**
   * @class MaterialSolver
   * @brief This class solves material expressions
   */
  class MaterialSolver {
  public:
    
    //std::unique_ptr<exs::Solver<Substance>> solver;  ///< Pointer to the EXS solver object

    /**
     * @brief Constructor of this class
     */
    MaterialSolver();    
  };

} // namespace snt::mat

#endif // MAT_MATERIAL_SOLVER_H
