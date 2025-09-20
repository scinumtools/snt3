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
  
} // namespace snt

#endif // SNT_SETTINGS_H
