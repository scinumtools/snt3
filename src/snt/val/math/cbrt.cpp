#include <snt/val/math/cbrt.h>

namespace snt::val::math {

  val::BaseValue::PointerType cbrt(val::BaseValue::PointerType a) {
    return a->math_cbrt();
  }

} // namespace snt::val::math
