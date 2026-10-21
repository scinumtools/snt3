#include <fstream>
#include <iterator>
#include <snt/api/dip_format.h>
#include <snt/dip/cursor.h>
#include <snt/dip/dip.h>
#include <stdexcept>

namespace snt::api {

    dip::tools::FormatOptions dip_format_options(const std::filesystem::path& input) {
        const auto manifest = dip::nearest_project_manifest(input);
        if (!manifest)
            return {};
        const auto project = dip::parse_project_manifest(*manifest);
        dip::tools::FormatOptions options;
        options.blank_lines_between_nodes = project["format.blank_lines_between_nodes"].as<int>();
        options.final_newline = project["format.final_newline"].as<bool>();
        return options;
    }

    std::string format_dip_file(const std::filesystem::path& input) {
        std::ifstream stream(input, std::ios::binary);
        if (!stream)
            throw std::runtime_error("Cannot read DIPL file: " + input.string());
        const auto source = std::string(std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{});
        return dip::tools::format(source, dip_format_options(input));
    }

} // namespace snt::api
