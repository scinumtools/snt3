#ifndef MAT_SUBSTANCE_H
#define MAT_SUBSTANCE_H

#include "component.h"
#include "composite.h"
#include "matter.h"

namespace snt::mat {

  /**
   * @class Substance
   * @brief A form of matter with uniform and definite composition and distinct physical and chemical properties.
   */
  class Substance: public Composite, public Component<double>, public Matter {
  public:
  };

} // namespace snt::mat

#endif // MAT_SUBSTANCE_H
