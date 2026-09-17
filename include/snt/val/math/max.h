#ifndef VAL_MATH_MAX_H
#define VAL_MATH_MAX_H

#include <snt/val/value_base.h>

namespace snt::val::math {

    /** Apply the max operation to this operand.
     * @param a1 First operand.
     * @param a2 Second operand.
     */
    extern val::BaseValue::PointerType max(val::BaseValue::PointerType a1, val::BaseValue::PointerType a2);

} // namespace snt::val::math

#endif // VAL_MATH_MAX_H
