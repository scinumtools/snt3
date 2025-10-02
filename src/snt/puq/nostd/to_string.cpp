#include "to_string.h"

#include <string>

namespace snt::nostd {

  std::string to_string(const MAGNITUDE_PRECISION& value, int precision) {
    std::stringstream ss;
    ss << std::setprecision(precision);
    ss << value << std::scientific;
    return ss.str();
  }

#if defined(MAGNITUDE_ARRAYS)
  std::string to_string(const puq::Array& value, int precision) {
    return value.to_string(precision);
  }
#elif defined(MAGNITUDE_VALUES)
  std::string to_string(val::BaseValue::PointerType value, int precision) {
    snt::StringFormatType fmt;
    fmt.valuePrecision = precision;
    return value->to_string(fmt);
  }
#endif

#ifdef MAGNITUDE_ERRORS
  std::string to_string(const puq::Magnitude& value, int precision) {
    return value.to_string(precision);
  }
#endif

  std::string to_string(const puq::BaseUnits& value, int precision) {
    return value.to_string();
  }

  std::string to_string(const puq::Dimensions& value, const puq::UnitFormat& format) {
    return value.to_string(format);
  }

  std::string to_string(const puq::Exponent& value, const puq::UnitFormat& format) {
    return value.to_string(format);
  }

  std::string to_string(const bool& use_prefixes, const puq::AllowedPrefixes& value) {
    std::stringstream ss;
    if (use_prefixes) {
      if (value.size() > 0) {
        std::stringstream sss;
        bool sep = false;
        for (auto& pref : value) {
          sss << (sep ? ", " : "") << pref;
          sep = true;
        }
        ss << sss.str();
      } else
        ss << "all";
    }
    return ss.str();
  }

#if defined(MAGNITUDE_ARRAYS)
  std::string to_string(const puq::ArrayShape& shape) {
    std::stringstream ss;
    ss << "[";
    for (int i = 0; i < shape.size(); i++) {
      if (i > 0)
        ss << ",";
      ss << std::to_string(shape[i]);
    }
    ss << "]";
    return ss.str();
  }
#elif defined(MAGNITUDE_VALUES)
  std::string to_string(const val::Array::ShapeType& shape) {
    std::stringstream ss;
    ss << "[";
    for (int i = 0; i < shape.size(); i++) {
      if (i > 0)
        ss << ",";
      ss << std::to_string(shape[i]);
    }
    ss << "]";
    return ss.str();
  }
#endif

} // namespace snt::nostd
