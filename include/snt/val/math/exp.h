#ifndef VAL_MATH_EXP_H
#define VAL_MATH_EXP_H

#include <snt/val/value_base.h>

namespace snt::val::math {

    /** Apply the exp operation to this operand.
     * @param e Input operand.
     */
    extern val::BaseValue::PointerType exp(val::BaseValue::PointerType e);

} // namespace snt::val::math

#endif // VAL_MATH_EXP_H
