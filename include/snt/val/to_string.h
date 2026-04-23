#ifndef VAL_MATH_TO_STRING_H
#define VAL_MATH_TO_STRING_H

#include <snt/val/value_base.h>
#include <string>

namespace snt::val {

  extern std::string to_string(val::BaseValue::PointerType value, int precision = std::cout.precision());

} // namespace snt::val

#endif // VAL_MATH_TO_STRING_H
