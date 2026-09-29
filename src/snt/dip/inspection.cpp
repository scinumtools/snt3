#include <snt/dip/inspection.h>

#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>

#include <stdexcept>
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
    return {name, node->value->get_dtype(), node->value->get_shape(), node->value->clone(),
            node->units, node->metadata, node->tags, provenance, declaration, replacement,
            env.get_applied_schemas(name), env.get_contributing_schema(name)};
}

std::vector<ValueInspection> inspect_values(const Environment& env) {
    std::vector<ValueInspection> result;
    result.reserve(env.nodes.size());
    for (const auto& node : env.nodes.get_nodes()) {
        if (node && node->value) result.push_back(inspect_value(env, node->path.name));
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
