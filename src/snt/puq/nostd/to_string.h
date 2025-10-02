#ifndef NOSTD_TO_STRING_H
#define NOSTD_TO_STRING_H

#include "nostd.h"

namespace snt::nostd {

  extern std::string to_string(const MAGNITUDE_PRECISION& value, int precision = std::cout.precision());
#ifdef EXPONENT_FRACTIONS
  extern std::string to_string(const puq::Exponent& value, const puq::UnitFormat& format = puq::UnitFormat());
#endif
#if defined(MAGNITUDE_ARRAYS)
  extern std::string to_string(const Array& value, int precision = std::cout.precision());
#elif defined(MAGNITUDE_VALUES)
  extern std::string to_string(val::BaseValue::PointerType value, int precision = std::cout.precision());
#endif
#ifdef MAGNITUDE_ERRORS
  extern std::string to_string(const puq::Magnitude& value, int precision = std::cout.precision());
#endif
  extern std::string to_string(const puq::BaseUnits& value, int precision = std::cout.precision());
  extern std::string to_string(const puq::Dimensions& value, const puq::UnitFormat& format = puq::UnitFormat());
  extern std::string to_string(const bool& use_prefixes, const puq::AllowedPrefixes& value);
#if defined(MAGNITUDE_ARRAYS)
  extern std::string to_string(const ArrayShape& shape);
#endif

} // namespace snt::nostd

#endif // NOSTD_TO_STRING_H
