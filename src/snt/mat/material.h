#ifndef MAT_MATERIAL_H
#define MAT_MATERIAL_H

#include "composite.h"
#include "matter.h"

namespace snt::mat {

  /**
   * @class Material
   * @brief Any substance or mixture of substances that can be used to make objects or structures
   */
  class Material: public Composite, public Matter {
  public:
  };

} // namespace snt::mat

#endif // MAT_MATERIAL_H
