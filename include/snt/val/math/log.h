#ifndef VAL_MATH_LOG_H
#define VAL_MATH_LOG_H

#include <snt/val/value_base.h>

namespace snt::val::math {

    /** Apply the log operation to this operand.
     * @param a Input operand.
     */
    extern val::BaseValue::PointerType log(val::BaseValue::PointerType a);

} // namespace snt::val::math

#endif // VAL_MATH_LOG_H
