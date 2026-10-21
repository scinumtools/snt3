#ifndef SNT_API_DIP_VALIDATE_H
#define SNT_API_DIP_VALIDATE_H

#include <filesystem>
#include <snt/dip/tools/validator.h>
#include <string>

namespace snt::api {

    /** Validate a DIPL source file or DIPfile project through the native parser. */
    dip::tools::ValidationResult validate_dip_file(const std::filesystem::path& input, bool project = false);

    /** Serialize one validation result for command-line and agent clients. */
    std::string dip_validation_json(const std::filesystem::path& input, const dip::tools::ValidationResult& result);

} // namespace snt::api

#endif
