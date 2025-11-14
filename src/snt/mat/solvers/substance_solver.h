#ifndef MAT_SUBSTANCE_SOLVER_H
#define MAT_SUBSTANCE_SOLVER_H

#include "../../exs.h"
#include "element_atom.h"

namespace snt::mat {

  /**
   * @class SubstanceSolver
   * @brief This class solves substance expressions
   */
  class SubstanceSolver {
  public:
    
    std::unique_ptr<exs::Solver<Element>> solver;  ///< Pointer to the EXS solver object

    /**
     * @brief Constructor of this class
     */
    SubstanceSolver();    
  };

} // namespace snt::mat

#endif // MAT_SUBSTANCE_SOLVER_H
