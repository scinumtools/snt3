#ifndef MAT_MATTER_H
#define MAT_MATTER_H

#include "../puq.h"

namespace snt::mat {

  /**
   * @class Matter
   * @brief Matter is anything that has mass and occupies space
   */
  class Matter {
  public:
    puq::Quantity mass_density;     ///< Mass density
    puq::Quantity number_density;   ///< Number density
    puq::Quantity volume;           ///< Volume
    puq::Quantity mass;             ///< Mass
    
  };

} // namespace snt::mat

#endif // MAT_MATTER_H
