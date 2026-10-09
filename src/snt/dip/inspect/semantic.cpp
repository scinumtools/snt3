#include <snt/dip/inspect/inspector.h>

#include "value_facts.h"

#include <snt/core/datatypes.h>
#include <snt/core/string_format.h>

#include <limits>
#include <stdexcept>

namespace snt::dip {
namespace {
std::string type_name(core::DataType type) {
    const auto found = core::DataTypeNames.find(type);
    return found == core::DataTypeNames.end() ? "unknown" : found->second;
}
} // namespace

SemanticDescription Inspector::describe(std::string_view path, std::size_t max_value_elements) const {
    const auto& env = *env_;
    SemanticDescription result;
    result.path = std::string(path);
    const auto capabilities = this->capabilities(path);
    result.dependencies_recorded = this->graph().recorded;
    result.kind = capabilities.hasValue ? "value" : capabilities.hasTabularData ? "table" : "group";
    if (!capabilities.hasValue && !capabilities.hasTabularData &&
        env.hierarchy.has_collection(result.path)) {
        const auto kind = env.hierarchy.get_collection(result.path).kind;
        if (kind == Path::Kind::Map || kind == Path::Kind::List) result.kind = "collection";
    }
    if (!capabilities.hasValue) {
        result.value_unavailable_reason = "not_a_value";
        return result;
    }
    const auto facts = detail::inspect_value_facts(env, result.path, true);
    result.declared_type = type_name(facts.declared_type);
    result.stored_type = type_name(facts.stored_type);
    result.shape = facts.shape;
    result.elements = facts.elements;
    if (facts.scalar_dimension && result.elements == 1) result.shape.clear();
    if (max_value_elements != 0 && result.elements <= max_value_elements) {
        core::StringFormatType format;
        format.valuePrecision = std::numeric_limits<double>::max_digits10;
        auto text = facts.value->to_string(format);
        if (text.size() <= 4096) result.value_text = std::move(text);
        else result.value_unavailable_reason = "omitted_by_byte_limit";
    } else {
        result.value_unavailable_reason = "omitted_by_limit";
    }
    if (facts.units) result.units = facts.units->to_string();
    result.metadata = facts.metadata;
    result.tags = facts.tags;
    result.overridden = facts.overridden;
    result.declaration = facts.declaration_location;
    result.override_location = facts.override_location;
    result.source_text_available = facts.source_text_available;
    result.enforced_condition = facts.enforced_condition;
    result.enforced_options = facts.enforced_options;
    if (result.dependencies_recorded)
        result.dependencies = this->graph().dependencies("?" + result.path);
    return result;
}

SemanticList Inspector::list_descriptions(const std::string& query, const TagFilter& tags,
                                          std::size_t limit, std::size_t max_value_elements) const {
    SemanticList result;
    if (query.empty() || query.front() != '?')
        throw std::invalid_argument("Description lists require an evaluated path query beginning with ?.");
    const auto paths = select_paths(query, tags);
    result.total = paths.size();
    for (const auto& path : paths) {
        if (result.items.size() >= limit) break;
        result.items.push_back(describe(path, max_value_elements));
    }
    return result;
}
} // namespace snt::dip
