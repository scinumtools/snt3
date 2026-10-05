#ifndef SNT_DIP_PREVIEW_H
#define SNT_DIP_PREVIEW_H

#include <snt/core/diagnostic.h>
#include <snt/dip/comparison.h>

#include <filesystem>
#include <string>
#include <vector>

namespace snt::dip {

struct PreviewOverride {
    enum class Kind { Text, File };
    Kind kind = Kind::Text;
    std::string input;
};

struct PreviewResult {
    bool baseline_valid = false;
    bool candidate_valid = false;
    std::vector<core::Diagnostic> baseline_diagnostics;
    std::vector<core::Diagnostic> candidate_diagnostics;
    std::vector<std::string> accepted_override_targets;
    ComparisonResult comparison;
};

/** Evaluate independent baseline and candidate parses without writing outputs. */
PreviewResult preview(const std::filesystem::path& input, const std::vector<PreviewOverride>& overrides,
                      bool record_dependency_graph = false,
                      const ComparisonOptions& options = {});

} // namespace snt::dip

#endif
