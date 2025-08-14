#ifndef SNT_SETTINGS_H
#define SNT_SETTINGS_H

#include <string_view>

namespace snt {

  // Keywords
  constexpr std::string_view KEYWORD_TRUE = "true";
  constexpr std::string_view KEYWORD_FALSE = "false";

  // Various settings
  constexpr int DISPLAY_FLOAT_PRECISION = 4;

} // namespace snt

#endif // SNT_SETTINGS_H
