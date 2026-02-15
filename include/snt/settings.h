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

#define CHECKPOINT(msg) \
    do { \
        std::cout << "[" << __LINE__ \
                  << ", " << file_basename(__FILE__) \
                  << "] " << msg << '\n'; \
    } while (0)

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
