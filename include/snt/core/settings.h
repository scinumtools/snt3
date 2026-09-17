#ifndef SNT_SETTINGS_H
#define SNT_SETTINGS_H

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <vector>

/** Return the final component of a slash- or backslash-separated path. */
constexpr const char* file_basename(const char* path) {
    const char* last = path;
    for (const char* p = path; *p; ++p)
        if (*p == '/' || *p == '\\')
            last = p + 1;
    return last;
}

/** Print a source location and optional diagnostic values to standard output. */
#define setCheckpoint(...)                                                                                             \
    do {                                                                                                               \
        std::cout << "[" << file_basename(__FILE__) << ":" << __LINE__ << "]";                                         \
        if constexpr (sizeof(#__VA_ARGS__) > 1)                                                                        \
            std::cout << " " << __VA_ARGS__;                                                                           \
        std::cout << '\n';                                                                                             \
    } while (0)

namespace snt::core {

    /// Keyword used for an absent value.
    constexpr std::string_view KEYWORD_NONE = "none";
    /// Keyword used for a true Boolean value.
    constexpr std::string_view KEYWORD_TRUE = "true";
    /// Keyword used for a false Boolean value.
    constexpr std::string_view KEYWORD_FALSE = "false";
    /// Opening delimiter used for array rendering.
    constexpr std::string_view SYMBOL_ARRAY_START = "[";
    /// Closing delimiter used for array rendering.
    constexpr std::string_view SYMBOL_ARRAY_END = "]";
    /// Element delimiter used for array rendering.
    constexpr std::string_view SYMBOL_ARRAY_DELIMITER = ", ";

} // namespace snt::core

// Windows specific settings
#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

#endif // SNT_SETTINGS_H
