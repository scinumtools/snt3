#include <snt/api/dip_semantic.h>

#include <snt/dip/artifact.h>
#include <snt/dip/inspect/inspector.h>

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string_view>

namespace snt::api {
namespace {
std::string quote(std::string_view value) {
    std::ostringstream out;
    out << '"';
    for (unsigned char c : value) {
        switch (c) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\b': out << "\\b"; break;
        case '\f': out << "\\f"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (c < 0x20) out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << int(c) << std::dec;
            else out << char(c);
        }
    }
    out << '"';
    return out.str();
}

void strings(std::ostream& out, const std::vector<std::string>& values) {
    out << '[';
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i) out << ',';
        out << quote(values[i]);
    }
    out << ']';
}

void location(std::ostream& out, const std::optional<core::SourceLocation>& value) {
    if (!value) { out << "null"; return; }
    out << "{\"source\":" << quote(value->source) << ",\"line\":" << value->line;
    if (value->column) out << ",\"column\":" << *value->column;
    out << '}';
}

void metadata(std::ostream& out, const dip::ValueMetadata& value) {
    out << "{\"description\":" << quote(value.description)
        << ",\"authors\":" << quote(value.authors)
        << ",\"title\":" << quote(value.title)
        << ",\"journal\":" << quote(value.journal)
        << ",\"year\":" << quote(value.year)
        << ",\"volume\":" << quote(value.volume)
        << ",\"issue\":" << quote(value.issue)
        << ",\"pages\":" << quote(value.pages)
        << ",\"doi\":" << quote(value.doi)
        << ",\"url\":" << quote(value.url)
        << ",\"version\":" << quote(value.version)
        << ",\"created\":" << quote(value.created)
        << ",\"modified\":" << quote(value.modified)
        << ",\"license\":" << quote(value.license)
        << ",\"rationale\":" << quote(value.rationale)
        << ",\"recommended_range\":" << quote(value.recommended_range)
        << ",\"performance_impact\":" << quote(value.performance_impact)
        << ",\"scientific_impact\":" << quote(value.scientific_impact)
        << ",\"deprecated\":" << quote(value.deprecated)
        << ",\"replacement\":" << quote(value.replacement)
        << ",\"since\":" << quote(value.since)
        << ",\"category\":" << quote(value.category)
        << ",\"visibility\":" << quote(value.visibility)
        << ",\"native\":"; strings(out, value.native);
    out << ",\"requires\":"; strings(out, value.requires);
    out << ",\"conflicts\":"; strings(out, value.conflicts);
    out << ",\"implies\":"; strings(out, value.implies);
    out << ",\"see\":"; strings(out, value.see);
    out << ",\"example\":"; strings(out, value.example);
    out << '}';
}

void description(std::ostream& out, const dip::SemanticDescription& value) {
    out << "{\"schema_version\":\"1\",\"path\":" << quote(value.path)
        << ",\"kind\":" << quote(value.kind)
        << ",\"declared_type\":" << quote(value.declared_type)
        << ",\"stored_type\":" << quote(value.stored_type) << ",\"shape\":[";
    for (std::size_t i = 0; i < value.shape.size(); ++i) {
        if (i) out << ',';
        out << value.shape[i];
    }
    out << "],\"elements\":" << value.elements << ",\"value\":";
    if (value.value_text)
        out << "{\"available\":true,\"encoding\":\"dipl\",\"text\":" << quote(*value.value_text) << '}';
    else
        out << "{\"available\":false,\"reason\":" << quote(value.value_unavailable_reason) << '}';
    out << ",\"units\":";
    if (value.units) out << quote(*value.units); else out << "null";
    out << ",\"metadata\":"; metadata(out, value.metadata);
    out << ",\"tags\":"; strings(out, value.tags);
    out << ",\"overridden\":" << (value.overridden ? "true" : "false")
        << ",\"default\":{\"available\":false,\"reason\":\"not_separately_evaluated\"}"
        << ",\"rules\":{\"options\":"; strings(out, value.enforced_options);
    out << ",\"condition\":";
    if (value.enforced_condition.empty()) out << "null";
    else out << quote(value.enforced_condition);
    out << "},\"dependencies\":{\"available\":" << (value.dependencies_recorded ? "true" : "false");
    if (value.dependencies_recorded) {
        out << ",\"reads\":[";
        for (std::size_t i = 0; i < value.dependencies.size(); ++i) {
            if (i) out << ',';
            out << "{\"target\":" << quote(value.dependencies[i].target)
                << ",\"request\":" << quote(value.dependencies[i].request) << '}';
        }
        out << ']';
    } else out << ",\"reason\":\"not_recorded\"";
    out << "},\"source_text\":{\"available\":"
        << (value.source_text_available ? "true" : "false");
    if (!value.source_text_available) out << ",\"reason\":\"not_retained\"";
    out << "},\"provenance\":{\"declaration\":";
    location(out, value.declaration);
    out << ",\"override\":";
    location(out, value.override_location);
    out << "}}";
}

void diagnostic(std::ostream& out, const core::Diagnostic& value) {
    out << "{\"code\":" << quote(value.code)
        << ",\"message\":" << quote(value.message)
        << ",\"details\":" << quote(value.details)
        << ",\"suggestion\":" << quote(value.suggestion)
        << ",\"node_path\":";
    if (value.node_path) out << quote(*value.node_path); else out << "null";
    out << ",\"location\":";
    location(out, value.location);
    out << '}';
}

void diagnostics(std::ostream& out, const std::vector<core::Diagnostic>& values) {
    out << '[';
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i) out << ',';
        diagnostic(out, values[i]);
    }
    out << ']';
}
} // namespace

const dip::Inspector& DIPSemantic::inspector() const {
    if (!inspector_) {
        auto environment = std::make_unique<dip::Environment>(dip::open_artifact(input_, record_dependency_graph_));
        auto inspector = std::make_unique<dip::Inspector>(*environment);
        environment_ = std::move(environment);
        inspector_ = std::move(inspector);
    }
    return *inspector_;
}

void DIPSemantic::reload() {
    auto environment = std::make_unique<dip::Environment>(dip::open_artifact(input_, record_dependency_graph_));
    auto inspector = std::make_unique<dip::Inspector>(*environment);
    inspector_ = std::move(inspector);
    environment_ = std::move(environment);
}

std::string DIPSemantic::describe_json(const std::string& path, std::size_t max_value_elements) const {
    std::ostringstream out;
    description(out, inspector().describe(path, max_value_elements));
    return out.str();
}

std::string DIPSemantic::override_contract_json(const std::string& path) const {
    const auto contract = inspector().override_contract(path);
    const char* kind = "unavailable";
    switch (contract.kind) {
    case dip::OverrideTargetKind::ExistingValue: kind = "existing_value"; break;
    case dip::OverrideTargetKind::ExistingItem: kind = "existing_item"; break;
    case dip::OverrideTargetKind::NewItem: kind = "new_item"; break;
    case dip::OverrideTargetKind::Unavailable: break;
    }
    std::ostringstream out;
    out << "{\"schema_version\":\"1\",\"path\":" << quote(contract.path)
        << ",\"kind\":" << quote(kind) << ",\"reason\":";
    if (contract.reason.empty()) out << "null"; else out << quote(contract.reason);
    out << ",\"resolved_path\":";
    if (contract.resolved_path.empty()) out << "null"; else out << quote(contract.resolved_path);
    out << ",\"declared_type\":";
    if (contract.declared_type == core::DataType::None) out << "null";
    else out << quote(core::DataTypeNames.at(contract.declared_type));
    out << ",\"current_shape\":";
    if (contract.current_shape) {
        out << '[';
        for (std::size_t i = 0; i < contract.current_shape->size(); ++i) {
            if (i) out << ',';
            out << contract.current_shape->at(i);
        }
        out << ']';
    } else out << "null";
    out << ",\"units\":";
    if (contract.units) out << quote(*contract.units); else out << "null";
    out << ",\"rules\":{\"options\":";
    strings(out, contract.enforced_options);
    out << ",\"condition\":";
    if (contract.enforced_condition.empty()) out << "null";
    else out << quote(contract.enforced_condition);
    out << "},\"item_schemas\":";
    strings(out, contract.item_schemas);
    out << ",\"snapshot_input\":" << (contract.snapshot_input ? "true" : "false")
        << ",\"requires_preview\":true}";
    return out.str();
}

std::string DIPSemantic::list_json(const std::string& query, const dip::TagFilter& tags,
                                   std::size_t limit, std::size_t max_value_elements) const {
    const auto result = inspector().list_descriptions(query, tags, limit, max_value_elements);
    std::ostringstream out;
    out << "{\"schema_version\":\"1\",\"total\":" << result.total
        << ",\"truncated\":" << (result.items.size() < result.total ? "true" : "false")
        << ",\"items\":[";
    for (std::size_t i = 0; i < result.items.size(); ++i) {
        if (i) out << ',';
        description(out, result.items[i]);
    }
    out << "]}";
    return out.str();
}

std::string DIPSemantic::preview_json(const std::vector<dip::PreviewOverride>& overrides,
                                      std::size_t max_details) const {
    const auto result = dip::preview(input_, overrides, record_dependency_graph_);
    std::ostringstream out;
    out << "{\"schema_version\":\"1\",\"baseline_valid\":"
        << (result.baseline_valid ? "true" : "false")
        << ",\"candidate_valid\":" << (result.candidate_valid ? "true" : "false")
        << ",\"baseline_diagnostics\":";
    diagnostics(out, result.baseline_diagnostics);
    out << ",\"candidate_diagnostics\":";
    diagnostics(out, result.candidate_diagnostics);
    out << ",\"accepted_override_targets\":";
    strings(out, result.accepted_override_targets);
    const auto& differences = result.comparison.differences;
    if (!result.baseline_valid || !result.candidate_valid) {
        out << ",\"diff\":{\"available\":false,\"reason\":\"evaluation_failed\"}}";
        return out.str();
    }
    out << ",\"diff\":{\"available\":true,\"added\":" << result.comparison.added
        << ",\"removed\":" << result.comparison.removed
        << ",\"changed\":" << result.comparison.changed
        << ",\"total\":" << differences.size()
        << ",\"truncated\":" << (differences.size() > max_details ? "true" : "false")
        << ",\"differences\":[";
    for (std::size_t i = 0; i < std::min(max_details, differences.size()); ++i) {
        const auto& difference = differences[i];
        if (i) out << ',';
        const char* kind = difference.kind == dip::DifferenceKind::Added ? "added" :
                           difference.kind == dip::DifferenceKind::Removed ? "removed" : "changed";
        out << "{\"path\":" << quote(difference.path)
            << ",\"category\":" << quote(difference.category)
            << ",\"kind\":" << quote(kind)
            << ",\"fields\":";
        strings(out, difference.fields);
        out << ",\"before\":" << quote(difference.before)
            << ",\"after\":" << quote(difference.after)
            << ",\"changed_elements\":" << difference.changed_elements
            << '}';
    }
    out << "]}}";
    return out.str();
}
} // namespace snt::api
