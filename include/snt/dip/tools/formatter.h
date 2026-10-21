#ifndef SNT_DIP_TOOLS_FORMATTER_H
#define SNT_DIP_TOOLS_FORMATTER_H

#include <string>
#include <string_view>

namespace snt::dip::tools {

    /** Style options defined by the built-in DIPfile format schema. */
    struct FormatOptions {
        int blank_lines_between_nodes = 1; ///< Blank lines between sibling node blocks.
        bool final_newline = true;         ///< Whether output ends with one newline.
    };

    /** Format DIPL source text without evaluating values or changing node order. */
    std::string format(std::string_view source, const FormatOptions& options = {});

} // namespace snt::dip::tools

#endif
