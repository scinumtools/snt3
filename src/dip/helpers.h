#ifndef H_HELPERS
#define H_HELPERS

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <string>
#include <string_view>

namespace dip {

  // trim whitespace from the beginning and the end of a string
  inline void trim(std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(), ::isspace);
    auto end = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
    str = (start < end) ? std::string(start, end) : "";
  }

  // concatenate multiple strings at the compile time
  // this is used to compose regular expression patterns
  template <size_t N>
  consteval std::array<char, N> ce_to_array(const char* str) {
    std::array<char, N> arr = {};
    constexpr size_t len = N - 1;
    for (size_t i = 0; i < len; ++i) {
      arr[i] = str[i];
      if (str[i] == '\0')
        break;
    }
    return arr;
  }
  template <size_t N>
  consteval std::array<char, N> ce_to_array(std::string_view str) {
    std::array<char, N> arr = {};
    constexpr size_t len = N - 1;
    for (size_t i = 0; i < len && i < str.size(); ++i) {
      if (i == str.size()) {
        arr[i] = '\0';
        break;
      } else {
        arr[i] = str[i];
      }
    }
    return arr;
  }
  template <size_t N>
  consteval std::array<char, N> ce_to_array(char str) {
    return {str, '\0'};
  }
  template <size_t N>
  consteval std::array<char, N> ce_to_array(std::array<char, N> str) {
    return str;
  }
  template <size_t N, typename First, typename Second, typename... Rest>
  consteval std::array<char, N> ce_concat(First first, Second second, Rest... rest) {
    std::array<char, N> first_array = ce_to_array<N>(first);
    std::array<char, N> second_array = ce_to_array<N>(second);
    size_t first_len = 0;
    size_t second_len = 0;
    while (first_array[first_len] != '\0')
      ++first_len;
    while (second_array[second_len] != '\0')
      ++second_len;
    size_t total_len = first_len + second_len;
    for (size_t i = 0; i < second_len; i++)
      first_array[first_len + i] = second_array[i];
    first_array[total_len] = '\0';
    if constexpr (sizeof...(rest) > 0)
      return ce_concat<N>(first_array, rest...);
    else
      return first_array;
  }

} // namespace dip

#endif // H_HELPERS
