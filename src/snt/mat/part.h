#ifndef MAT_PART_H
#define MAT_PART_H

#include "../puq.h"

#include <string>

namespace snt::mat {

  /**
   * @class Part
   * @brief A part is an individual substance or part that combines with others to form a larger whole
   *
   * @tparam PROPORTION proportion coefficient type (double or integer)
   */
  template <typename PROPORTION>
  class Part {
  protected:
    PROPORTION proportion;        ///< Proportion of a componet in a composite (double or integer)
    std::string expression;       ///< String representation of a part
    puq::Quantity part_mass;      ///< Part mass
  public:

    /**
     * @brief Create a part
     *
     * @param prop Proportion of this part in a Composite 
     * @param expr String representation of a part
     */
    Part(PROPORTION prop, const std::string& expr = ""): proportion(prop), expression(expr) {};
  };

}  

#endif // MAT_PART_H
