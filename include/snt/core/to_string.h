#ifndef CORE_TO_STRING_H
#define CORE_TO_STRING_H

namespace snt::core {

    /** Format a floating-point value with the requested precision.
     *  @param value Value to format.
     *  @param precision Number of significant digits.
     *  @return Formatted value.
     */
    extern std::string to_string(const double value, int precision = std::cout.precision());

} // namespace snt::core

#endif // CORE_TO_STRING_H
