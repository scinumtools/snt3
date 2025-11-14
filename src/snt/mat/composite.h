#ifndef MAT_COMPOSITE_H
#define MAT_COMPOSITE_H

#include <string>

namespace snt::mat {

  /**
   * @class Composite
   * @brief A composite is a material or object made up of two or more distinct components that remain physically separate and identifiable within the final structure
   *
   * @tparam SOLVER Type of an expression solver used by a composite
   * @tparam COMPONENT Type of composite components
   */
  template <typename SOLVER, typename COMPONENT>
  class Composite {
  public:
    std::string expression;        ///< String representation of a composite
    bool natural;                  ///< true if composite is using natural abundances

    /**
     * @brief Composite class constructor
     *
     * @param expr Composite string representation
     * @param nat Indicate if substance atoms should use natural abundance
     */
    Composite(const std::string& expr, bool nat = true):
      expression(expr), natural(nat) {

      SOLVER solver;      
    };
    
  };

} // namespace snt::mat

#endif // MAT_COMPOSITE_H
