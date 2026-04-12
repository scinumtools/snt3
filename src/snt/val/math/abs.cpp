#include <snt/val/math/abs.h>

namespace snt::val::math {

  val::BaseValue::PointerType abs(val::BaseValue::PointerType a) {
    return a->math_abs();
  }

} // namespace snt::val::math
