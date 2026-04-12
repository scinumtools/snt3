#ifndef VAL_MATH_POW_H
#define VAL_MATH_POW_H

#include <snt/val/value_base.h>

namespace snt::val::math {

  extern val::BaseValue::PointerType pow(val::BaseValue::PointerType a, const double e);
  extern val::BaseValue::PointerType pow(val::BaseValue::PointerType a, val::BaseValue::PointerType e);

} // namespace snt::val::math

#endif // VAL_MATH_POW_H
