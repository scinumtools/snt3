#ifndef MAT_COMPONENT_H
#define MAT_COMPONENT_H

#include "../puq.h"

#include <string>

namespace snt::mat {

  /**
   * @class Component
   * @brief A component is an individual substance or part that combines with others to form a larger whole
   *
   * @tparam PROPORTION proportion coefficient type (double or integer)
   */
  template <typename PROPORTION>
  class Component {
  protected:
    PROPORTION proportion;        ///< Proportion of a componet in a composite (double or integer)
    std::string expression;       ///< String representation of a component
    puq::Quantity component_mass; ///< Component mass
  public:

    /**
     * @brief Create a component
     *
     * @param prop Proportion of this component in a Composite 
     * @param expr String representation of a component
     */
    Component(PROPORTION prop, const std::string& expr = ""): proportion(prop), expression(expr) {};
  };

}  

#endif // MAT_COMPONENT_H
