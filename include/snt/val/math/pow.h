#ifndef VAL_MATH_POW_H
#define VAL_MATH_POW_H

#include <snt/val/value_base.h>

namespace snt::val::math {

    /** Apply the pow operation to this operand.
     * @param a Input operand.
     * @param e Input operand.
     */
    extern val::BaseValue::PointerType pow(val::BaseValue::PointerType a, const double e);
    /** Apply the pow operation to this operand.
     * @param a Input operand.
     * @param e Input operand.
     */
    extern val::BaseValue::PointerType pow(val::BaseValue::PointerType a, val::BaseValue::PointerType e);

} // namespace snt::val::math

#endif // VAL_MATH_POW_H
