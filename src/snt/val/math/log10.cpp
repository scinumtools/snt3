#include <snt/val/math/log10.h>

namespace snt::val::math {

  val::BaseValue::PointerType log10(val::BaseValue::PointerType a) {
    return a->math_log10();
  }

} // namespace snt::val::math
