#include <snt/val/math/floor.h>

namespace snt::val::math {

  val::BaseValue::PointerType floor(val::BaseValue::PointerType a) {
    return a->math_floor();
  }

} // namespace snt::val::math
