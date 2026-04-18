#include <algorithm>
#include <iomanip>
#include <iostream>
#include <snt/puq/to_string.h>
#include <snt/puq/settings.h>
#include <snt/puq/value/dimensions.h>
#include <sstream>

namespace snt::puq {

  Dimensions::Dimensions() : utype(Utype::NUL), numerical(1) {
    for (int i = 0; i < Config::num_basedim; i++) {
      physical[i] = 0;
    }
  }

  Dimensions::Dimensions(const Result& n) : utype(Utype::NUL), numerical(n) {
    for (int i = 0; i < Config::num_basedim; i++) {
      physical[i] = 0;
    }
  }

  Dimensions::Dimensions(const double m, const double e) : utype(Utype::NUL), numerical(m, e) {
    for (int i = 0; i < Config::num_basedim; i++) {
      physical[i] = 0;
    }
  }

  inline std::string _numerical_to_string(Result numerical, const BaseDimensions& physical,
                                          const UnitFormat& format) {
    std::string multiply = format.multiply_symbol();
    std::stringstream ss;
    if (format.base == Format::Base::MKS) {
      numerical = numerical * (Result)(std::pow(1e-3, exponent_to_float(physical[1])));
    } else if (format.base == Format::Base::CGS) {
      numerical = numerical * (Result)(std::pow(1e2, exponent_to_float(physical[0])));
    }
    if (!numerical.estimate->is_unity() && format.display_result()) {
      ss << numerical.to_string(format) << multiply;
    }
    // if (numerical.estimate != 1 && format.display_result()) {
    //   ss << numerical.to_string(format) << multiply;
    // }
    return ss.str();
  }

  inline std::string _physical_to_string(const BaseDimensions& physical, const UnitFormat& format) {
    std::string multiply = format.multiply_symbol();
    std::stringstream ss;
    for (int i = 0; i < Config::num_basedim; i++) {
      std::string symbol = SystemData::BaseUnitOrder[i];
      if (i == 1 && format.base == Format::Base::MKS) {
        symbol = "kg";
      } else if (i == 0 && format.base == Format::Base::CGS) {
        symbol = "cm";
      }
      double fexp = exponent_to_float(physical[i]);
      if (fexp == 1)
        ss << symbol << multiply;
      else if (fexp != 0)
        ss << symbol << puq::to_string(physical[i], format) << multiply;
    }
    return ss.str();
  }

  std::string Dimensions::to_string(const UnitFormat& format) const {
    std::string multiply = format.multiply_symbol();
    std::stringstream ss;
    if (format.display_result()) {
      ss << _numerical_to_string(numerical, physical, format);
    }
    if (format.display_units()) {
      ss << _physical_to_string(physical, format);
    }
    std::string s = ss.str();
    s = s.substr(0, s.size() - multiply.size());
    return (s == "") ? "1" : s;
  }

  bool Dimensions::is_numeric() const {
    bool all_unity = true;
    for (int i = 0; i < Config::num_basedim; i++) {
      if (exponent_to_float(physical[i])!=0)
        all_unity = false;
    }
    return all_unity;
  }
  
  std::ostream& operator<<(std::ostream& os, const Dimensions& d) {
    os << d.to_string();
    return os;
  }

  bool Dimensions::operator==(const Dimensions& d) const {
    for (int i = 0; i < Config::num_basedim; i++) {
      if (physical[i] != d.physical[i])
        return false;
    }
    return true;
  }

  bool Dimensions::operator!=(const Dimensions& d) const {
    bool equal = false;
    for (int i = 0; i < Config::num_basedim; i++) {
      if (physical[i] != d.physical[i])
        return true;
    }
    return false;
  }

} // namespace snt::puq
