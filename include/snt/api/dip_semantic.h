#ifndef SNT_API_DIP_SEMANTIC_H
#define SNT_API_DIP_SEMANTIC_H

#include <snt/dip/inspect/semantic.h>
#include <snt/dip/preview.h>

#include <filesystem>
#include <string>
#include <vector>

namespace snt::api {

/** Application-facing entry points sharing one versioned JSON contract. */
class DIPSemantic {
  public:
    explicit DIPSemantic(std::filesystem::path input) : input_(std::move(input)) {}

    std::string describe_json(const std::string& path, std::size_t max_value_elements = 16,
                              bool record_dependency_graph = false) const;
    std::string list_json(const std::string& query = "?", const dip::TagFilter& tags = {},
                          std::size_t limit = 100, std::size_t max_value_elements = 0,
                          bool record_dependency_graph = false) const;
    std::string preview_json(const std::vector<dip::PreviewOverride>& overrides,
                             bool record_dependency_graph = false,
                             std::size_t max_details = 50) const;

  private:
    std::filesystem::path input_;
};

} // namespace snt::api

#endif
