#ifndef PUQ_DISPLAY_LENGTH_H
#define PUQ_DISPLAY_LENGTH_H

#include <iostream>
#include <string>

namespace snt::puq {

  static size_t display_length(const std::string& str) {
    size_t length = 0;

    for (size_t i = 0; i < str.size();) {
      unsigned char c = str[i];

      // Check if it's an ASCII character (single-byte)
      if ((c & 0x80) == 0) {
        // Single-byte ASCII character
        i += 1;
      }
      // Check for multi-byte characters
      else if ((c & 0xE0) == 0xC0) {
        // Two-byte character
        i += 2;
      } else if ((c & 0xF0) == 0xE0) {
        // Three-byte character
        i += 3;
      } else if ((c & 0xF8) == 0xF0) {
        // Four-byte character
        i += 4;
      } else {
        // Invalid UTF-8 byte, skip it (or handle as an error if preferred)
        i += 1;
        continue;
      }

      // Increment display length for each full character
      ++length;
    }

    return length;
  };

} // namespace snt::puq

#endif // PUQ_DISPLAY_LENGTH_H
