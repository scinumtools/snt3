#include "model.h"

#include <algorithm>

namespace snt::docs {

bool Publication::empty() const {
    return authors.empty() && title.empty() && journal.empty() && year.empty() && volume.empty() && issue.empty() &&
           pages.empty() && doi.empty() && url.empty();
}

namespace {
Publication publication_of(const dip::ValueMetadata& meta) {
    return {meta.authors, meta.title, meta.journal, meta.year, meta.volume, meta.issue, meta.pages, meta.doi, meta.url};
}

Origin origin_of(const std::string& name, size_t line, const std::string& code,
                 const std::optional<dip::SourceInfo>& source) {
    return {name, source ? source->path : "", code, line};
}
} // namespace

Document build_document(const dip::Environment& env, std::string input_label, std::string introduction_tex,
                        bool loaded_snapshot) {
    Document document;
    document.input_label = std::move(input_label);
    document.introduction_tex = std::move(introduction_tex);
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
                                     env.get_source_info(node->line.source.name));
        if (node->override)
            item.replacement = origin_of(node->override_line.source.name, node->override_line.source.line_number,
                                         node->override_line.code, env.get_source_info(node->override_line.source.name));
        for (const auto& schema : env.get_applied_schemas(item.path))
            item.applied_schemas.push_back(schema.name);
        if (const auto schema = env.get_contributing_schema(item.path))
            item.contributing_schema = schema->name;
        document.parameters.push_back(std::move(item));
    }
    std::sort(document.parameters.begin(), document.parameters.end(), [](const Parameter& a, const Parameter& b) {
        return a.path < b.path;
    });

    for (const auto& [path, collection] : env.hierarchy.get_collections()) {
        if (path.empty()) continue;
        Structure item;
        item.path = path;
        switch (collection.kind) {
        case dip::Path::Kind::Group: item.kind = "group"; break;
        case dip::Path::Kind::Map: item.kind = "map"; break;
        case dip::Path::Kind::List: item.kind = "list"; break;
        case dip::Path::Kind::Item: item.kind = "item"; break;
        default: continue;
        }
        item.schemas = collection.schemas;
        document.structure.push_back(std::move(item));
    }
    std::sort(document.structure.begin(), document.structure.end(), [](const Structure& a, const Structure& b) {
        return a.path < b.path;
    });

    for (const auto& info : env.get_schema_manifest()) {
        Schema schema;
        schema.name = info.name;
        schema.description = info.metadata.description;
        schema.origin = origin_of(info.source_name, info.source_line, "", info.source);
        schema.publication = publication_of(info.metadata);
        document.schemas.push_back(std::move(schema));
    }
    std::sort(document.schemas.begin(), document.schemas.end(), [](const Schema& a, const Schema& b) {
        return a.name < b.name;
    });

    for (const auto& info : env.get_source_manifest())
        document.sources.push_back({info.name, info.path, info.parent_name, info.hash_algorithm,
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

} // namespace snt::docs
