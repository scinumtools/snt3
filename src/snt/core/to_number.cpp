#include "to_number.h"

#include <typeinfo>

namespace snt::core {

  double to_number(const std::string& value) {
    if (typeid(double) == typeid(float))
      return std::stof(value);
    else if (typeid(double) == typeid(double))
      return std::stod(value);
    else if (typeid(double) == typeid(long double))
      return std::stold(value);
    else
      throw std::range_error("Unsupported magnitude precision.");
  }

} // namespace snt::core
