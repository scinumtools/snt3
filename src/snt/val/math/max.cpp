#include <snt/val/math/max.h>

namespace snt::val::math {

  val::BaseValue::PointerType max(val::BaseValue::PointerType a1, val::BaseValue::PointerType a2) {
    return a1->math_max(a2.get());
  }

} // namespace snt::val::math
