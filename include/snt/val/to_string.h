#ifndef VAL_MATH_TO_STRING_H
#define VAL_MATH_TO_STRING_H

#include <snt/val/value_base.h>
#include <string>

namespace snt::val {

    /** Format a VAL value.
     *  @param value Value to format.
     *  @param precision Number of significant digits.
     *  @return Formatted value.
     */
    extern std::string to_string(
        val::BaseValue::PointerType value, int precision = static_cast<int>(std::cout.precision())
    );

    /** Format an array shape.
     *  @param shape Dimensions of the array.
     *  @return Formatted shape.
     */
    extern std::string to_string(const val::Array::ShapeType& shape);

} // namespace snt::val

#endif // VAL_MATH_TO_STRING_H
