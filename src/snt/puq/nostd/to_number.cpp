#include "to_number.h"

namespace snt::nostd {

  puq::MagnitudeFloat to_number(const std::string& value) {
    if (typeid(puq::MagnitudeFloat) == typeid(float))
      return std::stof(value);
    else if (typeid(puq::MagnitudeFloat) == typeid(double))
      return std::stod(value);
    else if (typeid(puq::MagnitudeFloat) == typeid(long double))
      return std::stold(value);
    else
      throw std::range_error("Unsupported magnitude precision.");
  }

} // namespace snt::nostd
