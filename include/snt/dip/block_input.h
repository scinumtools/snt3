#ifndef SNT_DIP_BLOCK_INPUT_H
#define SNT_DIP_BLOCK_INPUT_H

#include <cstddef>
#include <string>

namespace snt::dip {

/** Original string literal parsed into an array or table during a live parse. */
struct BlockInput {
    enum class Kind { Array, Table };
    Kind kind = Kind::Array;
    std::string path;
    std::string code;
    std::string source_name;
    std::size_t source_line = 0;
};

} // namespace snt::dip

#endif
