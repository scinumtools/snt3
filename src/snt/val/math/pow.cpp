#include <snt/val/math/pow.h>

namespace snt::val::math {

  val::BaseValue::PointerType pow(val::BaseValue::PointerType a, val::BaseValue::PointerType e) {
    return a->math_pow(e.get());
  }

} // namespace snt::val::math
