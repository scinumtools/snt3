#ifndef VAL_MATH_LOG10_H
#define VAL_MATH_LOG10_H

#include <snt/val/value_base.h>

namespace snt::val::math {

    /** Apply the log10 operation to this operand.
     * @param a Input operand.
     */
    extern val::BaseValue::PointerType log10(val::BaseValue::PointerType a);

} // namespace snt::val::math

#endif // VAL_MATH_LOG10_H
