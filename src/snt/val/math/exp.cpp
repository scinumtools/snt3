#include <snt/val/math/exp.h>

namespace snt::val::math {

  val::BaseValue::PointerType exp(val::BaseValue::PointerType a) {
    return a->math_exp();
  }

} // namespace snt::val::math
