#ifndef MAT_SUBSTANCE_H
#define MAT_SUBSTANCE_H

#include "element.h"
#include "component.h"
#include "composite.h"
#include "matter.h"
#include "solvers/substance_solver.h"

namespace snt::mat {

  /**
   * @class Substance
   * @brief A form of matter with uniform and definite composition and distinct physical and chemical properties.
   */
  class Substance: public Composite<SubstanceSolver, Element>, public Component<double>, public Matter {
  public:

    /**
     * @brief Substance class constructor
     *
     * @param expr Element string representation
     * @param prop Component proportion in a Composite
     * @param nat Indicate if substance atoms should use natural abundance
     */
    Substance(const std::string& expr, double prop = 1, bool nat = true);
    
    /**
     * @brief Addition of two substances
     *
     * @param other Pointer of another substance
     */
    void math_add(Substance* other);
    
    /**
     * @brief Multiplication of two substances
     *
     * @param other Pointer of another substance
     */
    void math_multiply(Substance* other);
  };

} // namespace snt::mat

#endif // MAT_SUBSTANCE_H
