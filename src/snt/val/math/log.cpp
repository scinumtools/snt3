#include <snt/val/math/log.h>

namespace snt::val::math {

  val::BaseValue::PointerType log(val::BaseValue::PointerType a) {
    return a->math_log();
  }

} // namespace snt::val::math
