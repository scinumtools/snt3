#ifndef SNT_SETTINGS_H
#define SNT_SETTINGS_H

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <vector>

constexpr const char* file_basename(const char* path) {
  const char* last = path;
  for (const char* p = path; *p; ++p)
    if (*p == '/' || *p == '\\')
      last = p + 1;
  return last;
}

#define setCheckpoint(...)                                        \
    do                                                            \
    {                                                             \
        std::cout << "["                                          \
                  << file_basename(__FILE__)                      \
                  << ":" << __LINE__ << "]";                      \
        if constexpr (sizeof(#__VA_ARGS__) > 1)                   \
            std::cout << " " << __VA_ARGS__;                      \
        std::cout << '\n';                                        \
    } while (0)

namespace snt::core {

  // Keywords
  constexpr std::string_view KEYWORD_NONE = "none";
  constexpr std::string_view KEYWORD_TRUE = "true";
  constexpr std::string_view KEYWORD_FALSE = "false";
  constexpr std::string_view SYMBOL_ARRAY_START = "[";
  constexpr std::string_view SYMBOL_ARRAY_END = "]";
  constexpr std::string_view SYMBOL_ARRAY_DELIMITER = ", ";

} // namespace snt::core

#endif // SNT_SETTINGS_H
