#ifndef SNT_SETTINGS_H
#define SNT_SETTINGS_H

#include <string_view>
#include <type_traits>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <iostream>

namespace snt {

  // Keywords
  constexpr std::string_view KEYWORD_TRUE  = "true";
  constexpr std::string_view KEYWORD_FALSE = "false";

  // Various settings
  constexpr int DISPLAY_FLOAT_PRECISION  = 4;
  constexpr int DISPLAY_FLOAT_SCIENTIFIC = 3;

  /*
  struct NumberFormat {
    int VALUE_PRECISION = 4;
    int EXPONENT_PRECISION = 4;
    double THRESHOLD_SCIENTIFIC = 1e3;
  };
  */
  
  template <typename T>
  std::string number_to_string(const T& value, int precision = DISPLAY_FLOAT_PRECISION) {
    if (value==0)
      return "0";
    std::stringstream ss;
    int exp_val = std::floor(std::log10(std::fabs(value)));  // rounded exponent
    if (std::abs(exp_val) >= DISPLAY_FLOAT_SCIENTIFIC && std::is_floating_point<T>::value) {
      // if exponent is a floating point number larger or equal than DISPLAY_FLOAT_SCIENTIFIC
      T val_mag = value * std::pow(10, -exp_val);            // magnitude without x10^ part
      ss << std::defaultfloat << std::setprecision(precision) << val_mag;
      if (exp_val)
	ss << 'e' << exp_val;
    } else {
      // in all other cases
      ss << std::defaultfloat << std::setprecision(precision) << value;
    }
    return ss.str();
  }

  /*
  template <typename T>
  std::string number_to_string(const T& value, const T& error, int precision = DISPLAY_FLOAT_PRECISION) {
    if (error==0) {
      return number_to_string<T>(value, precision);
    } else {
      std::stringstream ss;
      int abs_val = std::fabs(value);
      if (std::is_floating_point<T>::value) {
	// if exponent is a floating point number larger or equal than DISPLAY_FLOAT_SCIENTIFIC
	int exp_val = std::floor(std::log10(abs_val));    // rounded exponent of a value
	T val_mag = value * std::pow(10, -exp_val);       // magnitude without x10^ part
	int exp_err = std::floor(std::log10(error));      // rounded exponent of an error
	int exp_diff = std::abs(exp_val - exp_err) + 1;   // difference between value and error exponents
	int val_err = static_cast<int>(std::round(error * std::pow(10, 1 - exp_err))); // error value
	ss << std::fixed << std::setprecision(exp_diff) << val_mag;
	ss << "(" << std::setw(2) << std::setfill(' ') << val_err << ")";      
	if (exp_val)
	  ss << 'e' << exp_val;
      } else {
	// in all other cases
	ss << std::defaultfloat << std::setprecision(precision) << value;
      }
      return ss.str();
    }
  }
  */
  
} // namespace snt

#endif // SNT_SETTINGS_H
