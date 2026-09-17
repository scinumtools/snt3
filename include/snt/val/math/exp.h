#ifndef VAL_MATH_EXP_H
#define VAL_MATH_EXP_H

#include <snt/val/value_base.h>

namespace snt::val::math {

    /** Return the element-wise exponential of a VAL value.
     * @param e Value whose exponential is evaluated.
     */
    extern val::BaseValue::PointerType exp(val::BaseValue::PointerType e);

} // namespace snt::val::math

#endif // VAL_MATH_EXP_H
