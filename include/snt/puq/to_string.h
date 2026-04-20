#ifndef PUQ_MATH_TO_STRING_H
#define PUQ_MATH_TO_STRING_H

#include <string>
#include <snt/puq/measurement.h>

namespace snt::puq {

  extern std::string to_string(const puq::Result& value, int precision = std::cout.precision());
  extern std::string to_string(const puq::BaseUnits& value, int precision = std::cout.precision());
  extern std::string to_string(const puq::Dimensions& value, const puq::UnitFormat& format = puq::UnitFormat());
  extern std::string to_string(const puq::ExponentVariant& value, const puq::UnitFormat& format = puq::UnitFormat());
  extern std::string to_string(const bool& use_prefixes, const puq::AllowedPrefixes& value);

} // namespace snt::puq

#endif // PUQ_MATH_TO_STRING_H
