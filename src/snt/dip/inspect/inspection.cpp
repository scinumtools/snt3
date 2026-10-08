#include <snt/dip/inspect/inspection.h>

#include "artifact_input.h"
#include "value_facts.h"

#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>

#include <algorithm>
#include <stdexcept>
#include <set>
#include <unordered_set>
#include <utility>

namespace snt::dip {
namespace {
ValueNode::PointerType inspection_node(const Environment& env, std::string_view path) {
    const std::string name(path);
    const auto separator = name.find('?');
    if (separator == std::string::npos || separator == 0)
        return env.get_node(separator == 0 ? name.substr(1) : name);
    const auto source = env.sources.entries().find(name.substr(0, separator));
    if (source == env.sources.entries().end())
        throw std::out_of_range("No named DIP source found: " + name);
    const std::string local_path = name.substr(separator + 1);
    for (const auto& node : source->second.nodes.get_nodes())
        if (node && node->path.name == local_path) return node;
    throw std::out_of_range("No DIP value found in named source: " + name);
}

std::optional<InspectedSourceLocation> resolve_source_location(
    const Environment& env, SourceLocationRole role, const std::string& logical_name,
    std::size_t logical_line, std::size_t modification_index = 0) {
    if (logical_name.empty() || logical_line == 0) return std::nullopt;
    const auto parsed = env.sources.entries().find(logical_name);
    const bool retained_text = parsed != env.sources.entries().end();
    std::string physical_name = logical_name;
    std::size_t physical_line = logical_line;
    const bool embedded = retained_text && parsed->second.embedded;
    if (embedded) {
        physical_name = parsed->second.parent.name;
        physical_line = parsed->second.parent.line_number;
        if (physical_name.empty() || physical_line == 0) {
            physical_name = logical_name;
            physical_line = logical_line;
        }
    }
    const auto info = env.get_source_info(physical_name);
    const SourceInfo source = info.value_or(SourceInfo{physical_name, {}, {}, 0, {}, {}});
    return InspectedSourceLocation{role, source, physical_line, logical_name, logical_line,
                                   modification_index, embedded, retained_text};
}

core::SourceLocation project_source_location(const std::optional<InspectedSourceLocation>& resolved,
                                             const std::string& fallback_name, std::size_t fallback_line,
                                             const std::string& code,
                                             const std::optional<SourceInfo>& fallback_info = std::nullopt) {
    if (!resolved) return {fallback_info && !fallback_info->path.empty() ? fallback_info->path : fallback_name,
                           fallback_line, code};
    const auto& source = resolved->source;
    return {source.path.empty() ? source.name : source.path, resolved->line,
            resolved->embedded_registration ? std::nullopt : std::optional<std::string>(code)};
}
} // namespace

ArtifactKind detect_artifact(const std::filesystem::path& path) {
    if (path.filename() == "DIPfile") return ArtifactKind::Project;
    const auto extension = path.extension().string();
    if (extension == ".dip" || extension == ".dipl") return ArtifactKind::DIPL;
    if (extension == ".dipt") return ArtifactKind::TableText;
    if (extension == ".diph5") return ArtifactKind::DIPH5;
    return ArtifactKind::Unknown;
}

Environment open_artifact(const std::filesystem::path& path, bool record_dependency_graph,
                          bool retain_block_inputs) {
    switch (detect_artifact(path)) {
    case ArtifactKind::Project:
    case ArtifactKind::DIPL: {
        DIP parser;
        detail::register_parse_input(parser, path);
        return parser.parse(record_dependency_graph, retain_block_inputs);
    }
    case ArtifactKind::DIPH5: {
        Environment env;
        env.load(path);
        return env;
    }
    case ArtifactKind::TableText:
        throw std::invalid_argument("A .dipt file requires a DIPL table declaration; it cannot be loaded alone.");
    case ArtifactKind::Unknown:
        throw std::invalid_argument("Unknown DIP artifact: " + path.string());
    }
    throw std::invalid_argument("Unknown DIP artifact.");
}

void detail::register_parse_input(DIP& parser, const std::filesystem::path& path) {
    switch (detect_artifact(path)) {
    case ArtifactKind::Project: parser.add_project(path); return;
    case ArtifactKind::DIPL: parser.add_file(path); return;
    case ArtifactKind::TableText:
        throw std::invalid_argument("A .dipt file requires a DIPL table declaration; it cannot be loaded alone.");
    case ArtifactKind::DIPH5:
        throw std::invalid_argument("A .diph5 snapshot cannot be registered as parser input.");
    case ArtifactKind::Unknown:
        throw std::invalid_argument("Unknown DIP artifact: " + path.string());
    }
    throw std::invalid_argument("Unknown DIP artifact.");
}

void reload_artifact(Environment& current, const std::filesystem::path& path,
                     bool record_dependency_graph, bool retain_block_inputs) {
    Environment fresh = open_artifact(path, record_dependency_graph, retain_block_inputs);
    current = std::move(fresh);
}

const std::map<std::string, BlockInput>& inspect_block_inputs(const Environment& env) {
    return env.block_inputs();
}

const BlockInput* inspect_block_input(const Environment& env, std::string_view path) {
    const auto& blocks = env.block_inputs();
    const auto found = blocks.find(std::string(path));
    return found == blocks.end() ? nullptr : &found->second;
}

std::vector<InspectedSourceLocation> inspect_source_locations(
    const Environment& env, const SourceEntity& entity) {
    std::vector<InspectedSourceLocation> locations;
    const auto add = [&](SourceLocationRole role, const std::string& logical_name,
                         std::size_t logical_line, std::size_t modification_index = 0) {
        if (auto resolved = resolve_source_location(env, role, logical_name, logical_line, modification_index))
            locations.push_back(std::move(*resolved));
    };

    switch (entity.kind) {
    case SourceEntityKind::NamedSource: {
        add(SourceLocationRole::Source, entity.name, 1);
        break;
    }
    case SourceEntityKind::Schema: {
        const auto schema = env.schemas.entries().find(entity.name);
        if (schema != env.schemas.entries().end()) {
            add(SourceLocationRole::Definition, schema->second.source_name,
                schema->second.source_line);
            add(SourceLocationRole::Registration, schema->second.registration_source_name,
                schema->second.registration_line);
        } else {
            const auto manifest = env.get_schema_manifest();
            const auto found = std::find_if(manifest.begin(), manifest.end(), [&](const auto& item) {
                return item.name == entity.name;
            });
            if (found != manifest.end())
                add(SourceLocationRole::Definition, found->source_name, found->source_line);
        }
        break;
    }
    case SourceEntityKind::Unit: {
        const auto unit = env.units.entries().find(entity.name);
        if (unit != env.units.entries().end())
            add(SourceLocationRole::Definition, unit->second.source_name, unit->second.source_line);
        break;
    }
    case SourceEntityKind::ProjectEntry: {
        if (entity.index < env.project_entries().size()) {
            const auto& entry = env.project_entries()[entity.index];
            add(SourceLocationRole::Registration, entry.source_name, entry.line);
        }
        break;
    }
    case SourceEntityKind::Path: {
        const auto source = env.sources.entries().find(entity.source_name);
        if (!entity.source_name.empty() && source == env.sources.entries().end()) break;
        const auto& nodes = entity.source_name.empty()
            ? env.nodes.get_nodes() : source->second.nodes.get_nodes();
        const auto found = std::find_if(nodes.begin(), nodes.end(), [&](const auto& node) {
            return node && node->path.name == entity.name;
        });
        if (found != nodes.end()) {
            const auto& node = *found;
            if (node->override)
                add(SourceLocationRole::Override, node->override_line.source.name,
                    node->override_line.source.line_number);
            for (std::size_t index = node->modification_lines.size(); index > 0; --index) {
                const auto& change = node->modification_lines[index - 1];
                add(SourceLocationRole::Modification, change.source.name,
                    change.source.line_number, index);
            }
            add(SourceLocationRole::Declaration, node->line.source.name,
                node->line.source.line_number);
            if (!node->table_path.empty()) {
                if (const auto table = env.declarations().find(node->table_path))
                    add(SourceLocationRole::Declaration, table->source.name,
                        table->source.line_number);
            }
        } else if (entity.source_name.empty()) {
            const auto declaration = env.declarations().find(entity.name);
            if (declaration)
                add(SourceLocationRole::Declaration, declaration->source.name,
                    declaration->source.line_number);
        } else {
            const auto declaration = source->second.declarations.find(entity.name);
            if (declaration)
                add(SourceLocationRole::Declaration, declaration->source.name,
                    declaration->source.line_number);
        }
        break;
    }
    }
    return locations;
}

detail::ValueFacts detail::inspect_value_facts(const Environment& env, std::string_view path,
                                               bool include_description_details) {
    const std::string name(path);
    const auto node = env.get_node(name);
    if (!node->value)
        throw std::invalid_argument("The DIP node has no evaluated value: " + name);
    const auto provenance = env[name].get_provenance();
    const auto declaration_source = resolve_source_location(
        env, SourceLocationRole::Declaration, provenance.source_name, provenance.source_line);
    const auto declaration = project_source_location(
        declaration_source, provenance.source_name, provenance.source_line, provenance.source_code,
        provenance.source);
    std::optional<core::SourceLocation> replacement;
    if (node->override) {
        const auto source = resolve_source_location(env, SourceLocationRole::Override,
            node->override_line.source.name, provenance.override_line);
        replacement = project_source_location(source, node->override_line.source.name,
                                              provenance.override_line, provenance.override_code,
                                              provenance.override_source);
    }
    std::vector<ValueChange> changes{{ValueChangeKind::Declaration, declaration}};
    for (const auto& line : node->modification_lines)
        changes.push_back({ValueChangeKind::Modification,
                           project_source_location(resolve_source_location(env, SourceLocationRole::Modification,
                               line.source.name, line.source.line_number), line.source.name,
                               line.source.line_number, line.code)});
    if (replacement)
        changes.push_back({ValueChangeKind::Override, *replacement});
    std::vector<std::string> options;
    if (include_description_details) {
        options.reserve(node->options.size());
        for (const auto& option : node->options)
            options.push_back(option.value_raw + (option.units_raw.empty() ? "" : " " + option.units_raw));
    }
    return {name, node->value_dtype, node->value->get_dtype(), node->value->get_shape(),
            node->value->get_size(), node->dimension.empty(), node->value.get(),
            node->units, node->metadata, node->tags,
            node->override, provenance, declaration, replacement, std::move(changes),
            declaration_source && declaration_source->source_text_available, node->table_path,
            node->condition, std::move(options)};
}

ValueInspection inspect_value(const Environment& env, std::string_view path) {
    const auto facts = detail::inspect_value_facts(env, path);
    return {facts.path, facts.stored_type, facts.shape, facts.value->clone(),
            facts.units, facts.metadata, facts.tags, facts.provenance,
            facts.declaration_location, facts.override_location,
            env.get_applied_schemas(facts.path), env.get_contributing_schema(facts.path),
            facts.table_path, facts.changes};
}

ValueSummary inspect_value_summary(const Environment& env, std::string_view path) {
    const auto node = inspection_node(env, path);
    if (!node->value)
        throw std::invalid_argument("The DIP node has no evaluated value: " + std::string(path));
    return {std::string(path), node->value->get_dtype(), node->value->get_shape(), node->value->get_size(),
            node->units, node->metadata, node->table_path};
}

std::vector<ValueInspection> inspect_values(const Environment& env) {
    std::vector<ValueInspection> result;
    result.reserve(env.nodes.size());
    for (const auto& node : env.nodes.get_nodes()) {
        if (node && node->value) result.push_back(inspect_value(env, node->path.name));
    }
    return result;
}

InspectionCapabilities inspect_capabilities(const Environment& env, std::string_view path) {
    const std::string name(path);
    InspectionCapabilities capabilities;
    const auto separator = name.find('?');
    if (separator != std::string::npos && separator > 0) {
        const auto source = env.sources.entries().find(name.substr(0, separator));
        if (source == env.sources.entries().end())
            throw std::out_of_range("No named DIP source found: " + name);
        const std::string local_path = name.substr(separator + 1);
        bool found = local_path.empty();
        for (const auto& node : source->second.nodes.get_nodes()) {
            if (!node) continue;
            if (node->path.name == local_path) {
                found = true;
                capabilities.hasValue = bool(node->value);
                capabilities.hasSource = !node->line.source.name.empty();
                capabilities.hasProvenance = capabilities.hasSource || node->override ||
                                             !node->modification_lines.empty();
                capabilities.hasArrayData = node->value && !node->dimension.empty();
            }
            if (node->path.name.compare(0, local_path.size() + 1, local_path + ".") == 0 ||
                node->path.name.compare(0, local_path.size() + 1, local_path + "[") == 0)
                capabilities.hasChildren = true;
        }
        if (!found && !capabilities.hasChildren)
            throw std::out_of_range("No evaluated DIP path found: " + name);
        const auto& graph = env.dependency_graph();
        const auto* value_event = graph.latest(name, DependencyEventKind::Value);
        const auto* condition_event = graph.latest(name, DependencyEventKind::Condition);
        capabilities.hasReferenceGraph =
            (value_event && (!value_event->reads.empty() || value_event->composition.has_value())) ||
            (condition_event && (!condition_event->reads.empty() || condition_event->composition.has_value())) ||
            !graph.referenced_by(name).empty();
        return capabilities;
    }
    bool found = env.hierarchy.has_collection(name);
    const std::string child_prefix = name + ".";
    const std::string item_prefix = name + "[";
    for (const auto& node : env.nodes.get_nodes()) {
        if (!node) continue;
        if (node->path.name == name) {
            found = true;
            capabilities.hasValue = bool(node->value);
            capabilities.hasSource = !node->line.source.name.empty();
            capabilities.hasProvenance = capabilities.hasSource || node->override ||
                                         !node->modification_lines.empty();
            capabilities.hasArrayData = node->value && !node->dimension.empty();
            const auto* value_event = env.dependency_graph().latest("?" + name, DependencyEventKind::Value);
            const auto* condition_event = env.dependency_graph().latest("?" + name, DependencyEventKind::Condition);
            capabilities.hasReferenceGraph =
                (value_event && (!value_event->reads.empty() || value_event->composition.has_value())) ||
                (condition_event && (!condition_event->reads.empty() || condition_event->composition.has_value())) ||
                !env.dependency_graph().referenced_by("?" + name).empty();
        }
        if (node->table_path == name) {
            found = true;
            capabilities.hasTabularData = true;
        }
        if (node->path.name.compare(0, child_prefix.size(), child_prefix) == 0 ||
            node->path.name.compare(0, item_prefix.size(), item_prefix) == 0)
            capabilities.hasChildren = true;
    }
    for (const auto& entry : env.hierarchy.get_collections()) {
        const auto& collection_path = entry.first;
        if (collection_path.compare(0, child_prefix.size(), child_prefix) == 0 ||
            collection_path.compare(0, item_prefix.size(), item_prefix) == 0)
            capabilities.hasChildren = true;
    }
    if (!found && !capabilities.hasChildren)
        throw std::out_of_range("No evaluated DIP path found: " + name);
    return capabilities;
}

DependencyNeighborhood inspect_dependency_neighborhood(const Environment& env, std::string_view path) {
    if (path.empty()) throw std::invalid_argument("A dependency neighborhood requires a path");
    DependencyNeighborhood result;
    result.id = path.front() == '?' || path.find('?') != std::string_view::npos
        ? std::string(path) : "?" + std::string(path);
    const auto& graph = env.dependency_graph();
    result.recorded = graph.recorded;
    if (!result.recorded) return result;

    const auto* value_event = graph.latest(result.id, DependencyEventKind::Value);
    const auto* condition_event = graph.latest(result.id, DependencyEventKind::Condition);
    if (value_event) result.value_event = *value_event;
    if (condition_event) result.condition_event = *condition_event;

    std::unordered_set<std::string> seen;
    const auto add_dependencies = [&](const DependencyEvent* event) {
        if (!event) return;
        for (const auto& edge : event->reads)
            if (seen.insert(edge.target).second)
                result.dependencies.push_back({edge.target, edge.request, edge.operand});
    };
    add_dependencies(value_event);
    add_dependencies(condition_event);

    for (const auto& reader : graph.referenced_by(result.id)) {
        DependencyNeighbor neighbor{reader, {}, {}};
        for (const auto& edge : graph.dependencies(reader)) {
            if (edge.target == result.id) {
                neighbor.request = edge.request;
                neighbor.operand = edge.operand;
                break;
            }
        }
        result.readers.push_back(std::move(neighbor));
    }
    return result;
}

TableInspection inspect_table(const Environment& env, std::string_view path) {
    TableInspection table;
    table.path = std::string(path);
    const std::string prefix = table.path + ".";
    for (const auto& node : env.nodes.get_nodes()) {
        if (!node || node->table_path != table.path) continue;
        if (!node->value)
            throw std::invalid_argument("The table column has no evaluated value: " + node->path.name);
        const auto shape = node->value->get_shape();
        if (shape.size() != 1 || (!table.columns.empty() && shape[0] != table.rows))
            throw std::invalid_argument("The table columns have inconsistent shapes: " + table.path);
        if (node->path.name.compare(0, prefix.size(), prefix) != 0)
            throw std::invalid_argument("The table column is outside its table: " + node->path.name);
        table.rows = shape[0];
        table.columns.push_back({node->table_column_index, node->path.name.substr(prefix.size()), node->path.name,
                                 node->value->get_dtype(), node->units, node->metadata});
    }
    if (table.columns.empty())
        throw std::out_of_range("No evaluated table found: " + table.path);
    std::stable_sort(table.columns.begin(), table.columns.end(),
                     [](const auto& a, const auto& b) { return a.index < b.index; });
    return table;
}

std::vector<TableInspection> inspect_tables(const Environment& env) {
    std::vector<TableInspection> result;
    std::set<std::string> seen;
    for (const auto& node : env.nodes.get_nodes()) {
        if (node && !node->table_path.empty() && seen.insert(node->table_path).second)
            result.push_back(inspect_table(env, node->table_path));
    }
    return result;
}

val::BaseValue::PointerType read_value_slice(
    const Environment& env, std::string_view path, const val::Array::RangeType& ranges) {
    const std::string name(path);
    const auto node = inspection_node(env, name);
    if (!node->value)
        throw std::invalid_argument("The DIP node has no evaluated value: " + name);
    const auto shape = node->value->get_shape();
    if (ranges.size() != shape.size())
        throw std::invalid_argument("The slice rank does not match the DIP value: " + name);
    for (size_t i = 0; i < ranges.size(); ++i) {
        if (shape[i] == 0)
            throw std::out_of_range("The slice is outside the DIP value: " + name);
        const auto end = ranges[i].dmax == val::Array::max_range ? shape[i] - 1 : ranges[i].dmax;
        if (ranges[i].dmin > end || end >= shape[i])
            throw std::out_of_range("The slice is outside the DIP value: " + name);
    }
    return node->value->slice(ranges);
}

} // namespace snt::dip
