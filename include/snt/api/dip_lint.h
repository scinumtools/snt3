#ifndef SNT_API_DIP_LINT_H
#define SNT_API_DIP_LINT_H

#include <filesystem>
#include <snt/dip/tools/linter.h>
#include <string>

namespace snt::api {

    /** Validate and lint a DIPL source file or DIPfile project. */
    dip::tools::LintResult lint_dip_file(const std::filesystem::path& input, bool project = false);

    /** Serialize input validity, lint cleanliness, and structured diagnostics. */
    std::string dip_lint_json(const std::filesystem::path& input, const dip::tools::LintResult& result);

} // namespace snt::api

#endif
