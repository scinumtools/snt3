#ifndef VAL_MATH_SQRT_H
#define VAL_MATH_SQRT_H

#include <snt/val/value_base.h>

namespace snt::val::math {

    /** Apply the sqrt operation to this operand.
     * @param a Input operand.
     */
    extern val::BaseValue::PointerType sqrt(val::BaseValue::PointerType a);

} // namespace snt::val::math

#endif // VAL_MATH_SQRT_H
