#include "nostd.h"

namespace puq {
  namespace nostd {

    std::string to_string(const MAGNITUDE_PRECISION& value, int precision) {
      std::stringstream ss;
      ss << std::setprecision(precision);
      ss << value << std::scientific;
      return ss.str();
    }

#if defined(MAGNITUDE_ARRAYS)
    std::string to_string(const Array& value, int precision) {
      return value.to_string(precision);
    }
#elif defined(MAGNITUDE_VALUES)
    std::string to_string(val::BaseValue::PointerType value, int precision) {
      return value->to_string(precision);
    }
#endif

#ifdef MAGNITUDE_ERRORS
    std::string to_string(const Magnitude& value, int precision) {
      return value.to_string(precision);
    }
#endif

    std::string to_string(const BaseUnits& value, int precision) {
      return value.to_string();
    }

    std::string to_string(const Dimensions& value, const UnitFormat& format) {
      return value.to_string(format);
    }

    std::string to_string(const Exponent& value, const UnitFormat& format) {
      return value.to_string(format);
    }

    std::string to_string(const bool& use_prefixes, const AllowedPrefixes& value) {
      std::stringstream ss;
      if (use_prefixes) {
        if (value.size() > 0) {
          std::stringstream sss;
          bool sep = false;
          for (auto pref : value) {
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
    std::string to_string(const ArrayShape& shape) {
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

  } // namespace nostd
} // namespace puq
