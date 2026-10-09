#ifndef SNT_API_DIP_SEMANTIC_H
#define SNT_API_DIP_SEMANTIC_H

#include <snt/dip/inspect/inspector.h>
#include <snt/dip/preview.h>

#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace snt::api {

/** Application-facing entry points sharing one versioned JSON contract.
 * Descriptions and lists reuse one evaluated input; preview evaluates separately.
 */
class DIPSemantic {
  public:
    explicit DIPSemantic(std::filesystem::path input, bool record_dependency_graph = false)
        : input_(std::move(input)), record_dependency_graph_(record_dependency_graph) {}

    std::string describe_json(const std::string& path, std::size_t max_value_elements = 16) const;
    std::string override_contract_json(const std::string& path) const;
    std::string list_json(const std::string& query = "?", const dip::TagFilter& tags = {},
                          std::size_t limit = 100, std::size_t max_value_elements = 0) const;
    std::string preview_json(const std::vector<dip::PreviewOverride>& overrides,
                             std::size_t max_details = 50) const;
    /** Reload the input after it changes; a failed reload preserves the current view. */
    void reload();

  private:
    const dip::Inspector& inspector() const;
    std::filesystem::path input_;
    bool record_dependency_graph_ = false;
    mutable std::unique_ptr<dip::Environment> environment_;
    mutable std::unique_ptr<dip::Inspector> inspector_;
};

} // namespace snt::api

#endif
