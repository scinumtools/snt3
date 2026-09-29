#include <snt/dip/inspection.h>

#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>

#include <algorithm>
#include <stdexcept>
#include <set>
#include <utility>

namespace snt::dip {
namespace {
core::SourceLocation location(const std::optional<SourceInfo>& info, const std::string& name,
                              size_t line, const std::string& code) {
    const std::string source = info && !info->path.empty() ? info->path : name;
    return {source, line, code};
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

Environment open_artifact(const std::filesystem::path& path) {
    switch (detect_artifact(path)) {
    case ArtifactKind::Project: {
        DIP parser;
        parser.add_project(path);
        return parser.parse();
    }
    case ArtifactKind::DIPL: {
        DIP parser;
        parser.add_file(path);
        return parser.parse();
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

void reload_artifact(Environment& current, const std::filesystem::path& path) {
    Environment fresh = open_artifact(path);
    current = std::move(fresh);
}

ValueInspection inspect_value(const Environment& env, std::string_view path) {
    const std::string name(path);
    const auto node = env.get_node(name);
    if (!node->value)
        throw std::invalid_argument("The DIP node has no evaluated value: " + name);
    const auto provenance = env[name].get_provenance();
    const auto declaration = location(provenance.source, provenance.source_name,
                                      provenance.source_line, provenance.source_code);
    std::optional<core::SourceLocation> replacement;
    if (node->override)
        replacement = location(provenance.override_source, node->override_line.source.name,
                               provenance.override_line, provenance.override_code);
    std::vector<ValueChange> changes{{ValueChangeKind::Declaration, declaration}};
    for (const auto& line : node->modification_lines)
        changes.push_back({ValueChangeKind::Modification,
                           location(env.get_source_info(line.source.name), line.source.name,
                                    line.source.line_number, line.code)});
    if (replacement)
        changes.push_back({ValueChangeKind::Override, *replacement});
    return {name, node->value->get_dtype(), node->value->get_shape(), node->value->clone(),
            node->units, node->metadata, node->tags, provenance, declaration, replacement,
            env.get_applied_schemas(name), env.get_contributing_schema(name), node->table_path,
            std::move(changes)};
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
    const auto node = env.get_node(name);
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
