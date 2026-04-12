#include <snt/puq/to_string.h>
#include <string>

namespace snt::puq {

  std::string to_string(const puq::Result& value, int precision) {
    return value.to_string(precision);
  }

  std::string to_string(const puq::BaseUnits& value, int precision) {
    return value.to_string();
  }

  std::string to_string(const puq::Dimensions& value, const puq::UnitFormat& format) {
    return value.to_string(format);
  }

  std::string to_string(const puq::ExponentVariant& value, const puq::UnitFormat& format) {
    if (std::holds_alternative<int>(value)) {
      return format.format_exponents(std::to_string(std::get<int>(value)));
    } else {
      return std::get<puq::Exponent>(value).to_string(format);
    }
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

} // namespace snt::puq
