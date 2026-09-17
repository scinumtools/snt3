#ifndef VAL_MATH_LOG_H
#define VAL_MATH_LOG_H

#include <snt/val/value_base.h>

namespace snt::val::math {

    /** Return the element-wise natural logarithm of a VAL value.
     * @param a Value whose logarithm is evaluated.
     */
    extern val::BaseValue::PointerType log(val::BaseValue::PointerType a);

} // namespace snt::val::math

#endif // VAL_MATH_LOG_H
