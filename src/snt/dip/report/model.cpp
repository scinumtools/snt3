#include "model.h"

#include <algorithm>

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

    for (const auto& node : env.nodes.get_nodes()) {
        Parameter item;
        item.path = node->path.name;
        item.value = node->value ? node->value->to_string() : "none";
        item.units = node->units ? node->units->to_string() : "";
        item.description = node->metadata.description;
        item.publication = publication_of(node->metadata);
        item.overridden = node->override;
        item.declaration = origin_of(node->line.source.name, node->line.source.line_number, node->line.code,
                                     env.get_source_info(node->line.source.name), source_root);
        if (node->override)
            item.replacement = origin_of(node->override_line.source.name, node->override_line.source.line_number,
                                         node->override_line.code, env.get_source_info(node->override_line.source.name),
                                         source_root);
        for (const auto& schema : env.get_applied_schemas(item.path))
            item.applied_schemas.push_back(schema.name);
        if (const auto schema = env.get_contributing_schema(item.path))
            item.contributing_schema = schema->name;
        document.parameters.push_back(std::move(item));
    }
    std::sort(document.parameters.begin(), document.parameters.end(), [](const Parameter& a, const Parameter& b) {
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
