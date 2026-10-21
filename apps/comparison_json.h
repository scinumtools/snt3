/** @file
 * @brief Shared structured comparison output for application protocols.
 */
#ifndef SNT_APP_COMPARISON_JSON_H
#define SNT_APP_COMPARISON_JSON_H

#include <algorithm>
#include <nlohmann/json.hpp>
#include <snt/dip/comparison.h>

namespace snt::app {
    /** Serialize a DIP comparison for REST and MCP, retaining complete counts.
     * @param result Comparison computed by the DIP API.
     * @param max_details Maximum differences to include; zero returns counts only.
     * @return Structured differences with the number omitted from this response.
     */
    inline nlohmann::json comparison_json(const dip::ComparisonResult& result, std::size_t max_details) {
        auto differences = nlohmann::json::array();
        const auto count = std::min(max_details, result.differences.size());
        for (std::size_t i = 0; i < count; ++i) {
            const auto& difference = result.differences[i];
            const auto kind = difference.kind == dip::DifferenceKind::Added     ? "added"
                              : difference.kind == dip::DifferenceKind::Removed ? "removed"
                                                                                : "changed";
            differences.push_back(
                {{"path", difference.path},
                 {"category", difference.category},
                 {"kind", kind},
                 {"fields", difference.fields},
                 {"before", difference.before},
                 {"after", difference.after},
                 {"changed_elements", difference.changed_elements},
                 {"example_indices", difference.example_indices}}
            );
        }
        return {
            {"scope", result.scope == dip::ComparisonScope::Full ? "full" : "effective"},
            {"equal", result.equal()},
            {"added", result.added},
            {"removed", result.removed},
            {"changed", result.changed},
            {"differences", std::move(differences)},
            {"omitted", result.differences.size() - count}
        };
    }
} // namespace snt::app
#endif
