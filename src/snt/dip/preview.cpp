#include <snt/dip/preview.h>

#include <snt/dip/dip.h>
#include <snt/dip/artifact.h>
#include <snt/dip/inspect/diagnostic.h>

#include "artifact_input.h"

#include <stdexcept>
#include <set>

namespace snt::dip {
namespace {
Environment evaluate(const std::filesystem::path& input, const std::vector<PreviewOverride>& overrides,
                     bool record_dependency_graph) {
    DIP parser;
    const auto kind = detect_artifact(input);
    if (kind != ArtifactKind::Project && kind != ArtifactKind::DIPL)
        throw std::invalid_argument("Preview requires a DIPfile, .dip, or .dipl input.");
    detail::register_parse_input(parser, input);
    for (const auto& entry : overrides) {
        if (entry.kind == PreviewOverride::Kind::File) parser.add_override_file(entry.input);
        else parser.add_override_string(entry.input);
    }
    return parser.parse(record_dependency_graph);
}
} // namespace

PreviewResult preview(const std::filesystem::path& input, const std::vector<PreviewOverride>& overrides,
                      bool record_dependency_graph, const ComparisonOptions& options) {
    PreviewResult result;
    Environment baseline;
    Environment candidate;
    try {
        baseline = evaluate(input, {}, record_dependency_graph);
        result.baseline_valid = true;
    } catch (const std::exception& error) {
        result.baseline_diagnostics.push_back(diagnostic_from_exception(error));
    }
    try {
        candidate = evaluate(input, overrides, record_dependency_graph);
        result.candidate_valid = true;
    } catch (const std::exception& error) {
        result.candidate_diagnostics.push_back(diagnostic_from_exception(error));
    }
    if (result.baseline_valid && result.candidate_valid) {
        std::set<std::pair<std::string, std::string>> baseline_overrides;
        for (const auto& node : baseline.nodes.get_nodes())
            if (node && node->override)
                baseline_overrides.emplace(node->path.name, node->override_line.source.name);
        for (const auto& node : candidate.nodes.get_nodes())
            if (node && node->override &&
                !baseline_overrides.count({node->path.name, node->override_line.source.name}))
                result.accepted_override_targets.push_back(node->path.name);
        result.comparison = compare(baseline, candidate, options);
    }
    return result;
}
} // namespace snt::dip
