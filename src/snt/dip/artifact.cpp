#include <snt/dip/artifact.h>

#include "artifact_input.h"

#include <snt/dip/dip.h>

#include <stdexcept>
#include <utility>

namespace snt::dip {

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

} // namespace snt::dip
