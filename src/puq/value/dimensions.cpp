#include "../nostd/nostd.h"
#include "../settings.h"
#include "value.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace puq {

  Dimensions::Dimensions() : utype(Utype::NUL), numerical(1) {
    for (int i = 0; i < NUM_BASEDIM; i++) {
      physical[i] = 0;
    }
  }

  Dimensions::Dimensions(const MAGNITUDE_TYPE& n) : utype(Utype::NUL), numerical(n) {
    for (int i = 0; i < NUM_BASEDIM; i++) {
      physical[i] = 0;
    }
  }

#ifdef MAGNITUDE_ERRORS

  Dimensions::Dimensions(const MAGNITUDE_PRECISION& m, const MAGNITUDE_PRECISION& e) : utype(Utype::NUL), numerical(m, e) {
    for (int i = 0; i < NUM_BASEDIM; i++) {
      physical[i] = 0;
    }
  }

#endif

  inline std::string _numerical_to_string(MAGNITUDE_TYPE numerical, const BaseDimensions& physical,
                                          const UnitFormat& format) {
    std::string multiply = format.multiply_symbol();
    std::stringstream ss;
    if (format.base == Format::Base::MKS) {
      numerical = numerical * (MAGNITUDE_TYPE)(std::pow(1e-3, (EXPONENT_REAL_PRECISION)physical[1]));
    } else if (format.base == Format::Base::CGS) {
      numerical = numerical * (MAGNITUDE_TYPE)(std::pow(1e2, (EXPONENT_REAL_PRECISION)physical[0]));
    }
#if defined(MAGNITUDE_ERRORS)
#if defined(MAGNITUDE_VALUES)
    if (!numerical.value->is_unity() && format.display_magnitude()) {
      ss << numerical.to_string(format) << multiply;
    }
#else
    if (numerical.value != 1 && format.display_magnitude()) {
      ss << numerical.to_string(format) << multiply;
    }
#endif
#elif defined(MAGNITUDE_ARRAYS)
    if (numerical != 1 && format.display_magnitude()) {
      ss << numerical.to_string(format) << multiply;
    }
#elif defined(MAGNITUDE_VALUES)
    if (!numerical->is_unity() && format.display_magnitude()) {
      ss << numerical->to_string(format) << multiply;
    }
#else
    if (numerical != 1 && format.display_magnitude()) {
      ss << std::setprecision(format.precision);
      ss << numerical << std::scientific << multiply;
    }
#endif
    return ss.str();
  }

  inline std::string _physical_to_string(const BaseDimensions& physical, const UnitFormat& format) {
    std::string multiply = format.multiply_symbol();
    std::stringstream ss;
    for (int i = 0; i < NUM_BASEDIM; i++) {
      std::string symbol = SystemData::BaseUnitOrder[i];
      if (i == 1 && format.base == Format::Base::MKS) {
        symbol = "kg";
      } else if (i == 0 && format.base == Format::Base::CGS) {
        symbol = "cm";
      }
      if (physical[i] == 1)
        ss << symbol << multiply;
      else if (physical[i] != 0)
        ss << symbol << nostd::to_string(physical[i], format) << multiply;
    }
    return ss.str();
  }

  std::string Dimensions::to_string(const UnitFormat& format) const {
    std::string multiply = format.multiply_symbol();
    std::stringstream ss;
    if (format.display_magnitude()) {
      ss << _numerical_to_string(numerical, physical, format);
    }
    if (format.display_units()) {
      ss << _physical_to_string(physical, format);
    }
    std::string s = ss.str();
    s = s.substr(0, s.size() - multiply.size());
    return (s == "") ? "1" : s;
  }

  std::ostream& operator<<(std::ostream& os, const Dimensions& d) {
    os << d.to_string();
    return os;
  }

  bool Dimensions::operator==(const Dimensions& d) const {
    for (int i = 0; i < NUM_BASEDIM; i++) {
      if (physical[i] != d.physical[i])
        return false;
    }
    return true;
  }

  bool Dimensions::operator!=(const Dimensions& d) const {
    bool equal = false;
    for (int i = 0; i < NUM_BASEDIM; i++) {
      if (physical[i] != d.physical[i])
        return true;
    }
    return false;
  }

} // namespace puq
