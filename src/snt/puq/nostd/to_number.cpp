#include "to_number.h"

namespace snt::nostd {

  MAGNITUDE_PRECISION to_number(const std::string& value) {
    if (typeid(MAGNITUDE_PRECISION) == typeid(float))
      return std::stof(value);
    else if (typeid(MAGNITUDE_PRECISION) == typeid(double))
      return std::stod(value);
    else if (typeid(MAGNITUDE_PRECISION) == typeid(long double))
      return std::stold(value);
    else
      throw std::range_error("Unsupported magnitude precision.");
  }

} // namespace snt::nostd
