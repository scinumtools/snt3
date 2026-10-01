#include "model.h"

#include <algorithm>
#include <snt/dip/inspection.h>
#include <unordered_map>

namespace snt::dip::report {

namespace {
std::string display_path(const std::string& path, const std::filesystem::path& source_root) {
    if (path.empty() || source_root.empty()) return path;
    const auto relative = std::filesystem::path(path).lexically_relative(
        std::filesystem::absolute(source_root).lexically_normal());
    if (relative.empty() || *relative.begin() == "..") return path;
    return relative.generic_string();
}

Publication publication_of(const dip::ValueMetadata& meta) {
    return {meta.authors, meta.title, meta.journal, meta.year, meta.volume, meta.issue, meta.pages, meta.doi, meta.url};
}

Origin origin_of(const std::string& name, size_t line, const std::string& code,
                 const std::optional<dip::SourceInfo>& source, const std::filesystem::path& source_root) {
    return {name, source ? display_path(source->path, source_root) : "", code, line};
}

std::string type_name(core::DataType type) {
    const auto found = core::DataTypeNames.find(type);
    return found == core::DataTypeNames.end() ? "unknown" : found->second;
}

std::string shape_of(const dip::ValueNode& node) {
    if (node.dimension.empty() || !node.value) return "";
    std::string result;
    for (const auto extent : node.value->get_shape()) {
        if (!result.empty()) result += " x ";
        result += std::to_string(extent);
    }
    return result;
}
} // namespace

Document build_document(const dip::Environment& env, std::string input_label, std::string introduction_tex,
                        std::string title, std::string author, std::string date, std::string version,
                        const std::filesystem::path& source_root, bool loaded_snapshot) {
    Document document;
    document.input_label = std::move(input_label);
    document.introduction_tex = std::move(introduction_tex);
    document.title = std::move(title);
    document.author = std::move(author);
    document.date = std::move(date);
    document.version = std::move(version);
    document.loaded_snapshot = loaded_snapshot;
    const auto& graph = env.dependency_graph();
    document.graph_recorded = graph.recorded;

    for (const auto& node : env.nodes.get_nodes()) {
        Parameter item;
        item.path = node->path.name;
        item.value = node->value ? node->value->to_string() : "none";
        item.units = node->units ? node->units->to_string() : "";
        item.type = type_name(node->value ? node->value->get_dtype() : node->value_dtype);
        item.shape = shape_of(*node);
        item.description = node->metadata.description;
        item.publication = publication_of(node->metadata);
        item.overridden = node->override;
        item.declaration = origin_of(node->line.source.name, node->line.source.line_number, node->line.code,
                                     env.get_source_info(node->line.source.name), source_root);
        if (node->override)
            item.replacement = origin_of(node->override_line.source.name, node->override_line.source.line_number,
                                         node->override_line.code, env.get_source_info(node->override_line.source.name),
                                         source_root);
        for (const auto& change : node->modification_lines)
            item.modifications.push_back(origin_of(change.source.name, change.source.line_number, change.code,
                                                   env.get_source_info(change.source.name), source_root));
        for (const auto& schema : env.get_applied_schemas(item.path))
            item.applied_schemas.push_back(schema.name);
        if (const auto schema = env.get_contributing_schema(item.path))
            item.contributing_schema = schema->name;
        if (graph.recorded) {
            const auto id = "?" + item.path;
            if (const auto* value = graph.latest(id, DependencyEventKind::Value)) {
                item.expression = value->expression;
                for (const auto& read : value->reads)
                    if (std::find(item.reads.begin(), item.reads.end(), read.target) == item.reads.end())
                        item.reads.push_back(read.target);
                for (const auto& decision_id : value->controlled_by) {
                    const auto* decision = graph.latest(decision_id, DependencyEventKind::Decision);
                    item.selected_by.push_back(decision && !decision->expression.empty()
                        ? decision->expression : decision_id);
                }
            }
            if (const auto* condition = graph.latest(id, DependencyEventKind::Condition))
                item.condition = condition->expression;
        }
        document.parameters.push_back(std::move(item));
    }
    std::sort(document.parameters.begin(), document.parameters.end(), [](const Parameter& a, const Parameter& b) {
        return a.path < b.path;
    });
    if (graph.recorded) {
        std::unordered_map<std::string, std::vector<std::string>> readers;
        for (const auto& item : document.parameters)
            for (const auto& target : item.reads)
                readers[target].push_back(item.path);
        for (auto& item : document.parameters)
            if (const auto found = readers.find("?" + item.path); found != readers.end())
                item.used_by = std::move(found->second);
    }

    for (const auto& inspected : dip::inspect_tables(env)) {
        Table table;
        table.path = inspected.path;
        table.rows = inspected.rows;
        for (const auto& column : inspected.columns)
            table.columns.push_back({column.name, type_name(column.type),
                                     column.units ? column.units->to_string() : ""});
        document.tables.push_back(std::move(table));
    }
    std::sort(document.tables.begin(), document.tables.end(), [](const Table& a, const Table& b) {
        return a.path < b.path;
    });

    for (const auto& info : env.get_schema_manifest()) {
        Schema schema;
        schema.name = info.name;
        schema.description = info.metadata.description;
        schema.origin = origin_of(info.source_name, info.source_line, "", info.source, source_root);
        schema.publication = publication_of(info.metadata);
        document.schemas.push_back(std::move(schema));
    }
    for (const auto& item : document.parameters) {
        if (item.contributing_schema.empty()) continue;
        const auto schema = std::find_if(document.schemas.begin(), document.schemas.end(), [&](const Schema& entry) {
            return entry.name == item.contributing_schema;
        });
        if (schema != document.schemas.end()) schema->supplied_parameters.push_back(item.path);
    }
    std::sort(document.schemas.begin(), document.schemas.end(), [](const Schema& a, const Schema& b) {
        return a.name < b.name;
    });

    for (const auto& info : env.get_source_manifest())
        document.sources.push_back({info.name, display_path(info.path, source_root), info.parent_name, info.hash_algorithm,
                                    info.hash, info.parent_line});
    std::sort(document.sources.begin(), document.sources.end(), [](const Source& a, const Source& b) {
        return a.name < b.name;
    });

    for (const auto& [name, unit] : env.units.entries())
        document.units.push_back({name, unit.definition});

    for (const auto& info : env.get_trace_manifest()) {
        if (info.kind == "function_value" || info.kind == "function_nodes")
            document.functions.push_back({info.name, info.kind == "function_value" ? "value" : "nodes"});
    }
    std::sort(document.functions.begin(), document.functions.end(), [](const Function& a, const Function& b) {
        if (a.name != b.name) return a.name < b.name;
        return a.kind < b.kind;
    });
    return document;
}

} // namespace snt::dip::report
