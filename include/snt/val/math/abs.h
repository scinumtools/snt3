#ifndef VAL_MATH_ABS_H
#define VAL_MATH_ABS_H

#include <snt/val/value_base.h>

namespace snt::val::math {

    /** Return the element-wise absolute value of a VAL value. */
    /** Apply the abs operation to this operand.
     * @param a Input operand.
     */
    extern val::BaseValue::PointerType abs(val::BaseValue::PointerType a);

} // namespace snt::val::math

#endif // VAL_MATH_ABS_H
