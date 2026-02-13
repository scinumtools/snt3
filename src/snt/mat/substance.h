#ifndef MAT_SUBSTANCE_H
#define MAT_SUBSTANCE_H

#include "element.h"
#include "part.h"
#include "set.h"
#include "matter.h"
#include "solvers/substance_solver.h"

namespace snt::mat {

  /**
   * @class Substance
   * @brief A form of matter with uniform and definite composition and distinct physical and chemical properties.
   */
  class Substance: public Set<Element, int>,
		   public Part<double>,
		   public Matter {
  public:
    
    /**
     * @brief Substance class constructor from a scalar number
     *
     * @param num Number
     */
    Substance(double num);

    /**
     * @brief Substance class constructor from a map of components and their proportions
     *
     * @param comp Map of components and their proportions
     * @param prop Component proportion in a Composite
     * @param nat Indicate if substance atoms should use natural abundance
     */
    Substance(const ElementMap& comp, double prop = 1, bool nat = true);
    
    /**
     * @brief Substance class constructor from an expression
     *
     * @param expr Element string representation
     * @param prop Component proportion in a Composite
     * @param nat Indicate if substance atoms should use natural abundance
     */
    Substance(const std::string& expr, double prop = 1, bool nat = true);
    
    /**
     * @brief Convert element to a string
     *
     * @return String representation of a substance
     */
    std::string to_string() const;
  };

} // namespace snt::mat

#endif // MAT_SUBSTANCE_H
