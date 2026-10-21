#ifndef SNT_API_DIP_VALIDATE_H
#define SNT_API_DIP_VALIDATE_H

#include <filesystem>
#include <optional>
#include <snt/dip/tools/validator.h>
#include <string>

namespace snt::api {

    /** Validate a DIPL source file or DIPfile project through the native parser. */
    dip::tools::ValidationResult validate_dip_file(const std::filesystem::path& input, bool project = false);

    /** Serialize one validation result for command-line and agent clients. */
    std::string dip_validation_json(const std::filesystem::path& input, const dip::tools::ValidationResult& result);

    /** Serialize diagnostics shared by validation and lint clients. */
    std::string dip_diagnostics_json(
        const std::filesystem::path& input, bool valid, const std::vector<core::Diagnostic>& diagnostics,
        std::optional<bool> clean = std::nullopt
    );

} // namespace snt::api

#endif
