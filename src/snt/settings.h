#ifndef SNT_SETTINGS_H
#define SNT_SETTINGS_H

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <vector>

namespace snt {

  // Keywords
  constexpr std::string_view KEYWORD_NONE = "none";
  constexpr std::string_view KEYWORD_TRUE = "true";
  constexpr std::string_view KEYWORD_FALSE = "false";
  constexpr std::string_view SYMBOL_ARRAY_START = "[";
  constexpr std::string_view SYMBOL_ARRAY_END = "]";
  constexpr std::string_view SYMBOL_ARRAY_DELIMITER = ", ";

  struct NumberFormatType {
    int valuePrecision = 4;
    int errorPrecision = 2;
    int thresholdScientific = 3;
  };

  // Implementation for value only
  
  template <typename T>
  std::string number_to_string(const T& value, const NumberFormatType& format = NumberFormatType()) {
    std::stringstream ss;
    if (value == 0) {
      return "0";
    } else {
      int exp_val = std::floor(std::log10(std::fabs(value))); // rounded exponent
      if (std::abs(exp_val) >= format.thresholdScientific && std::is_floating_point_v<T>) {
        // if exponent is a floating point number larger or equal than thresholdScientific
        T val_mag = value * std::pow(10, -exp_val); // magnitude without x10^ part
        ss << std::defaultfloat << std::setprecision(format.valuePrecision) << val_mag;
        if (exp_val)
          ss << 'e' << exp_val;
      } else {
        // in all other cases
        ss << std::defaultfloat << std::setprecision(format.valuePrecision) << value;
      }
    }
    return ss.str();
  }

  template <typename T>
  std::string _array_to_string(const std::vector<T>& value, const std::vector<size_t>& shape, const NumberFormatType& format, size_t& offset, size_t dim) {
    std::ostringstream oss;
    if (value.size() > 1)
      oss << SYMBOL_ARRAY_START;
    for (size_t i = 0; i < shape[dim]; i++) {
      if (i > 0)
        oss << SYMBOL_ARRAY_DELIMITER;
      if (dim + 1 < shape.size()) {
        oss << _array_to_string(value, shape, format, offset, dim + 1);
      } else {
        if constexpr (std::is_same_v<T, bool>) {
          oss << (value[offset] ? KEYWORD_TRUE : KEYWORD_FALSE);
        } else if constexpr (std::is_same_v<T, char>) {
          oss << value[offset];
        } else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
          oss << number_to_string(value[offset], format);
        } else if constexpr (std::is_same_v<T, std::string>) {
          oss << '\'' << value[offset] << '\'';
        } else {
          return "<unsupported_type>";
        }
        offset++;
      }
    }
    if (value.size() > 1)
      oss << SYMBOL_ARRAY_END;
    return oss.str();
  }

  template <typename T>
  std::string array_to_string(const std::vector<T>& value, const std::vector<size_t>& shape, const NumberFormatType& format = NumberFormatType()) {
    size_t offset = 0;
    return _array_to_string(value, shape, format, offset, 0);
  }

  // Implementation for value/error 
  
  template <typename T>
  std::string number_to_string(const T& value, const T& error, const NumberFormatType& format = NumberFormatType()) {
    std::stringstream ss;
    if (value == 0 && error == 0) {
      return "0";
    } else if (error == 0) {
      return number_to_string(value, format);
    } else {
      int exp_val = std::floor(std::log10(std::fabs(value))); // rounded exponent
      int exp_err = std::floor(std::log10(std::fabs(error)));
      int exp_diff = std::abs(exp_val - exp_err) - 1;
      if (std::is_integral_v<T>) {
        int prec = (exp_err) ? format.errorPrecision - 1 : 0;
        int val_err = static_cast<int>(error * std::pow(10, -exp_err + prec));
        T val_mag = value * std::pow(10, -exp_err + prec); // magnitude without x10^ part
        ss << std::fixed << std::setprecision(exp_diff) << val_mag;
        ss << "(" << std::setprecision(format.errorPrecision) << val_err << ")";
        if (exp_err - prec)
          ss << 'e' << exp_err - prec;
      } else if (std::is_floating_point_v<T>) {
        int val_err = static_cast<int>(std::round(error * std::pow(10, -exp_err - 1 + format.errorPrecision)));
        T val_mag = value * std::pow(10, -exp_val); // magnitude without x10^ part
        ss << std::fixed << std::setprecision(exp_diff + format.errorPrecision) << val_mag;
        ss << "(" << std::setprecision(format.errorPrecision) << val_err << ")";
        if (exp_val)
          ss << 'e' << exp_val;
      }
    }
    return ss.str();
  }
  
  template <typename T>
  std::string _array_to_string(const std::vector<T>& value, const std::vector<T>& error, const std::vector<size_t>& shape, const NumberFormatType& format, size_t& offset, size_t dim) {
    std::ostringstream oss;
    if (value.size() > 1)
      oss << SYMBOL_ARRAY_START;
    for (size_t i = 0; i < shape[dim]; i++) {
      if (i > 0)
        oss << SYMBOL_ARRAY_DELIMITER;
      if (dim + 1 < shape.size()) {
        oss << _array_to_string(value, error, shape, format, offset, dim + 1);
      } else {
        if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
          oss << number_to_string(value[offset], error[offset], format);
        } else {
          return "<unsupported_type>";
        }
        offset++;
      }
    }
    if (value.size() > 1)
      oss << SYMBOL_ARRAY_END;
    return oss.str();
  }

  template <typename T>
  std::string array_to_string(const std::vector<T>& value, const std::vector<T>& error, const std::vector<size_t>& shape, const NumberFormatType& format = NumberFormatType()) {
    size_t offset = 0;
    return _array_to_string(value, error, shape, format, offset, 0);
  }
  
} // namespace snt

#endif // SNT_SETTINGS_H
