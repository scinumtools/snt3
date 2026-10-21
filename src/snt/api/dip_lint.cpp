#include <snt/api/dip_lint.h>
#include <snt/api/dip_validate.h>

namespace snt::api {
    dip::tools::LintResult lint_dip_file(const std::filesystem::path& input, bool project) {
        return dip::tools::lint_file(input, project);
    }

    std::string dip_lint_json(const std::filesystem::path& input, const dip::tools::LintResult& result) {
        return dip_diagnostics_json(input, result.input_valid, result.diagnostics, result.clean());
    }
} // namespace snt::api
