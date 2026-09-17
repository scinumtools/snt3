#ifndef PUQ_MATH_TO_STRING_H
#define PUQ_MATH_TO_STRING_H

#include <snt/puq/measurement.h>
#include <string>

namespace snt::puq {

    /** Format a PUQ result. */
    extern std::string to_string(const puq::Result& value, int precision = static_cast<int>(std::cout.precision()));
    /** Format a base-unit expression. */
    extern std::string to_string(const puq::BaseUnits& value, int precision = static_cast<int>(std::cout.precision()));
    /** Format dimensions using unit-format options. */
    extern std::string to_string(const puq::Dimensions& value, const puq::UnitFormat& format = puq::UnitFormat());
    /** Format an exponent using unit-format options. */
    extern std::string to_string(const puq::ExponentVariant& value, const puq::UnitFormat& format = puq::UnitFormat());
    /** Format an allowed-prefix map. */
    extern std::string to_string(const bool& use_prefixes, const puq::AllowedPrefixes& value);

} // namespace snt::puq

#endif // PUQ_MATH_TO_STRING_H
