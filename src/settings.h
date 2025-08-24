#ifndef SNT_SETTINGS_H
#define SNT_SETTINGS_H

#include <string_view>
#include <type_traits>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace snt {

  // Keywords
  constexpr std::string_view KEYWORD_TRUE  = "true";
  constexpr std::string_view KEYWORD_FALSE = "false";

  // Various settings
  constexpr int DISPLAY_FLOAT_PRECISION     = 4;
  constexpr double DISPLAY_FLOAT_SCIENTIFIC = 1e3;
  
  template <typename T>
  std::string number_to_string(const T& value, int precision = DISPLAY_FLOAT_PRECISION) {
    std::stringstream ss;
    int abs_val = std::fabs(value);
    if (abs_val >= DISPLAY_FLOAT_SCIENTIFIC && std::is_floating_point<T>::value) {
      // if exponent is a floating point number larger or equal than DISPLAY_FLOAT_SCIENTIFIC
      int exp_val = std::floor(std::log10(abs_val));
      T val_mag = value * std::pow(10, -exp_val);
      ss << std::defaultfloat << std::setprecision(precision) << val_mag << 'e' << exp_val;
    } else {
      // in all other cases
      ss << std::defaultfloat << std::setprecision(precision) << value;
    }
    return ss.str();
  }
  
} // namespace snt

#endif // SNT_SETTINGS_H
