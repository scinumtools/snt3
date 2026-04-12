#include <snt/val/math/sqrt.h>

namespace snt::val::math {

  val::BaseValue::PointerType sqrt(val::BaseValue::PointerType a) {
    return a->math_sqrt();
  }

} // namespace snt::val::math
