#ifndef NOSTD_TO_STRING_H
#define NOSTD_TO_STRING_H

#include <snt/puq/nostd/nostd.h>

namespace snt::nostd {

  extern std::string to_string(const double value, int precision = std::cout.precision());
  extern std::string to_string(val::BaseValue::PointerType value, int precision = std::cout.precision());
  extern std::string to_string(const puq::Result& value, int precision = std::cout.precision());
  extern std::string to_string(const puq::BaseUnits& value, int precision = std::cout.precision());
  extern std::string to_string(const puq::Dimensions& value, const puq::UnitFormat& format = puq::UnitFormat());
  extern std::string to_string(const puq::ExponentVariant& value, const puq::UnitFormat& format = puq::UnitFormat());
  extern std::string to_string(const bool& use_prefixes, const puq::AllowedPrefixes& value);

} // namespace snt::nostd

#endif // NOSTD_TO_STRING_H
