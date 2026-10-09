#ifndef SNT_DIP_INSPECT_SEMANTIC_H
#define SNT_DIP_INSPECT_SEMANTIC_H

#include <snt/dip/environment.h>
#include <snt/dip/inspect/inspection.h>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace snt::dip {

/** Bounded, owned description of one evaluated path. Values use DIPL text. */
struct SemanticDescription {
    std::string path;
    std::string kind;
    std::string declared_type;
    std::string stored_type;
    val::Array::ShapeType shape;
    std::size_t elements = 0;
    std::optional<std::string> value_text;
    std::string value_unavailable_reason;
    std::optional<std::string> units;
    ValueMetadata metadata;
    std::vector<std::string> tags;
    bool overridden = false;
    std::optional<core::SourceLocation> declaration;
    std::optional<core::SourceLocation> override_location;
    bool source_text_available = false;
    std::vector<std::string> enforced_options;
    std::string enforced_condition;
    bool dependencies_recorded = false;
    std::vector<DependencyEdge> dependencies;
};

struct SemanticList {
    std::vector<SemanticDescription> items;
    std::size_t total = 0;
};

} // namespace snt::dip

#endif
