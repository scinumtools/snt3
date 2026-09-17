#ifndef VAL_MATH_SQRT_H
#define VAL_MATH_SQRT_H

#include <snt/val/value_base.h>

namespace snt::val::math {

    /** Return the element-wise square root of a VAL value.
     * @param a Value whose square root is evaluated.
     */
    extern val::BaseValue::PointerType sqrt(val::BaseValue::PointerType a);

} // namespace snt::val::math

#endif // VAL_MATH_SQRT_H
