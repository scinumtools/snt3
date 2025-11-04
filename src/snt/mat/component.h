#ifndef MAT_COMPONENT_H
#define MAT_COMPONENT_H

#include "../puq.h"

#include <string>

namespace snt::mat {

  /**
   * @class Component
   * @brief A component is an individual substance or part that combines with others to form a larger whole
   *
   * @tparam P proportion coefficient type (double or integer)
   */
  template <typename P>
  class Component {
  protected:
    P proportion;                 ///< Proportion of a componet in a composite (double or integer)
    std::string expression;       ///< String representation of a component
    puq::Quantity component_mass; ///< Component mass
  public:

    /**
     * @brief Create a component
     *
     * @param expr String representation of a component
     * @param prop Proportion of this component in a Composite
     */
    Component(const std::string& expr, P prop): expression(expr), proportion(prop) {};
  };

}  

#endif // MAT_COMPONENT_H
