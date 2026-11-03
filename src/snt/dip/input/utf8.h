#pragma once

#include <string_view>

namespace snt::dip {
    /** Check text before it becomes DIPL source or archive metadata.
     *  This is deliberately byte-oriented: decoding would lose the exact source bytes
     *  that the archive hashes. Embedded NULs are rejected as well.
     *  @param text Bytes to inspect. @return True when they form valid UTF-8 without NULs. */
    inline bool valid_utf8(std::string_view text) {
        for (size_t i = 0; i < text.size();) {
            const auto first = static_cast<unsigned char>(text[i]);
            if (first == 0)
                return false;
            if (first < 0x80) {
                ++i;
                continue;
            }
            size_t continuation = 0;
            if (first >= 0xc2 && first <= 0xdf)
                continuation = 1;
            else if (first >= 0xe0 && first <= 0xef)
                continuation = 2;
            else if (first >= 0xf0 && first <= 0xf4)
                continuation = 3;
            else
                return false;
            if (i + continuation >= text.size())
                return false;
            const auto second = static_cast<unsigned char>(text[i + 1]);
            if ((second & 0xc0) != 0x80 || (first == 0xe0 && second < 0xa0) ||
                (first == 0xed && second >= 0xa0) || (first == 0xf0 && second < 0x90) ||
                (first == 0xf4 && second >= 0x90))
                return false;
            for (size_t offset = 2; offset <= continuation; ++offset)
                if ((static_cast<unsigned char>(text[i + offset]) & 0xc0) != 0x80)
                    return false;
            i += continuation + 1;
        }
        return true;
    }
} // namespace snt::dip
