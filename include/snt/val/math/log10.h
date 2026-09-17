#ifndef VAL_MATH_LOG10_H
#define VAL_MATH_LOG10_H

#include <snt/val/value_base.h>

namespace snt::val::math {

    /** Return the element-wise base-10 logarithm of a VAL value.
     * @param a Value whose logarithm is evaluated.
     */
    extern val::BaseValue::PointerType log10(val::BaseValue::PointerType a);

} // namespace snt::val::math

#endif // VAL_MATH_LOG10_H
