#ifndef MAT_SUBSTANCE_SOLVER_H
#define MAT_SUBSTANCE_SOLVER_H

#include "../../exs.h"
#include "../element.h"

namespace snt::mat {

  class Substance;
  
  /**
   * @class SubstanceSolver
   * @brief This class solves substance expressions
   */
  class SubstanceSolver {

    /**
     * @enum Parser state flags
     */
    enum class ParserState {
      START,
      ELEMENT,
      STATE,
      END,
      OPERATION,
      ERROR
    };
    
  public:
    
    std::unique_ptr<exs::Solver<Substance>> solver;  ///< Pointer to the EXS solver object

    /**
     * @brief Constructor of this class
     */
    SubstanceSolver();

    /**
     * @brief Preprocess expression for the solver
     *
     * @param expr Substance expression
     */
    std::string preprocess(const std::string& expr);

    /**
     * @brief Solve expression using expression solver
     *
     * @param expr Substance expression
     * @return Substance object
     */
    Substance solve(const std::string& expr);
  };

} // namespace snt::mat

#endif // MAT_SUBSTANCE_SOLVER_H
