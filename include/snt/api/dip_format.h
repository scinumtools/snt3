#ifndef SNT_API_DIP_FORMAT_H
#define SNT_API_DIP_FORMAT_H

#include <filesystem>
#include <snt/dip/tools/formatter.h>
#include <string>

namespace snt::api {

    /** Resolve the nearest DIPfile's schema-backed formatter options for a source file. */
    dip::tools::FormatOptions dip_format_options(const std::filesystem::path& input);

    /** Read and format a DIPL file while preserving its original values and ordering. */
    std::string format_dip_file(const std::filesystem::path& input);

} // namespace snt::api

#endif
