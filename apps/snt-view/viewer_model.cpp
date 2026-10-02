#include "viewer_model.h"

#include <algorithm>
#include <functional>
#include <utility>

namespace snt::view {
namespace {
std::string parent_path(const std::string& path) {
    if (!path.empty() && path.back() == ']') {
        const auto bracket = path.rfind('[');
        if (bracket != std::string::npos) return path.substr(0, bracket);
    }
    const auto dot = path.rfind('.');
    return dot == std::string::npos ? std::string{} : path.substr(0, dot);
}

std::string path_label(const std::string& path, const std::string& parent) {
    if (parent.empty()) return path;
    return path.substr(parent.size() + (path[parent.size()] == '.' ? 1 : 0));
}

std::string parent_id(const std::string& id) {
    if (id == "@project" || id == "@dipfile" || id == "@sources" || id == "@overrides" ||
        id == "@schemas" || id == "@units") return "";
    if (id.compare(0, 9, "@dipfile?") == 0) {
        const auto next = id.find('?', 9);
        return next == std::string::npos ? "@dipfile" : id.substr(0, next);
    }
    if (id.compare(0, 10, "@override?") == 0) return "@overrides";
    if (id.compare(0, 8, "@schema?") == 0) return "@schemas";
    if (id.compare(0, 6, "@unit?") == 0) return "@units";
    const auto question = id.find('?');
    if (question != std::string::npos) {
        const std::string source = id.substr(0, question + 1);
        const std::string parent = parent_path(id.substr(question + 1));
        return parent.empty() ? (id == source ? "@sources" : source) : source + parent;
    }
    const std::string parent = parent_path(id);
    return parent.empty() ? "@project" : parent;
}
} // namespace

ViewerModel::ViewerModel(std::filesystem::path artifact)
    : input_path_(artifact.string()), artifact_(std::filesystem::absolute(std::move(artifact))) {
    environment_ = dip::open_artifact(artifact_, true);
    rebuild_objects();
    history_.push_back(selection_);
}

std::string ViewerModel::display_file_path(const std::string& path) const {
    if (path.empty()) return {};
    const std::filesystem::path file(path);
    if (!file.is_absolute()) return file.generic_string();
    const auto relative = file.lexically_relative(artifact_.parent_path());
    return relative.empty() ? file.filename().generic_string() : relative.generic_string();
}

void ViewerModel::rebuild_objects() {
    objects_.clear();
    objects_.emplace("", ObjectInfo{"", artifact_.filename().string(), "", {}, false,
                                     dip::Path::Kind::None, ObjectRole::Artifact});
    objects_.emplace("@project", ObjectInfo{"@project", "Evaluated project", "", {}, false,
                                              dip::Path::Kind::None, ObjectRole::Project});
    objects_.at("").children.push_back("@project");
    if (dip::detect_artifact(artifact_) == dip::ArtifactKind::Project) {
        objects_.emplace("@dipfile", ObjectInfo{"@dipfile", "DIPfile", "", {}, false,
                                                  dip::Path::Kind::None, ObjectRole::DIPfile});
        for (std::size_t index = 0; index < environment_.project_entries().size(); ++index) {
            const auto& entry = environment_.project_entries()[index];
            const char* category = nullptr;
            switch (entry.kind) {
            case dip::ProjectEntry::Kind::Unit: category = "Units"; break;
            case dip::ProjectEntry::Kind::Source: category = "Sources"; break;
            case dip::ProjectEntry::Kind::Schema: category = "Schemas"; break;
            case dip::ProjectEntry::Kind::Code: category = "Code"; break;
            case dip::ProjectEntry::Kind::Override: category = "Overrides"; break;
            }
            const std::string category_id = std::string("@dipfile?") + category;
            if (objects_.find(category_id) == objects_.end()) {
                objects_.emplace(category_id, ObjectInfo{category_id, category, "@dipfile", {}, false,
                    dip::Path::Kind::None, ObjectRole::ManifestCategory});
                objects_.at("@dipfile").children.push_back(category_id);
            }
            const std::string id = category_id + "?" + std::to_string(index);
            const std::string label = entry.name.empty()
                ? (entry.resolved_path.empty() ? "Inline text" : entry.value) : entry.name;
            ObjectInfo item{id, label, category_id, {}, false, dip::Path::Kind::None,
                            ObjectRole::ManifestEntry};
            item.manifest_index = index;
            objects_.emplace(id, std::move(item));
            objects_.at(category_id).children.push_back(id);
        }
    }
    std::function<void(const std::string&, const std::string&)> ensure_object =
        [&](const std::string& path, const std::string& source) {
        if (path.empty()) return;
        const std::string id = source.empty() ? path : source + "?" + path;
        if (objects_.find(id) != objects_.end()) return;
        const std::string parent = parent_path(path);
        if (!parent.empty()) ensure_object(parent, source);
        const std::string parent_id = source.empty()
            ? (parent.empty() ? "@project" : parent)
            : source + "?" + parent;
        objects_.emplace(id, ObjectInfo{id, path_label(path, parent), parent_id, {}, false,
                                         dip::Path::Kind::None, ObjectRole::Path, source, path});
        objects_.at(parent_id).children.push_back(id);
    };
    for (const auto& node : environment_.nodes.get_nodes()) {
        if (!node) continue;
        const std::string& full = node->path.name;
        if (full.empty()) continue;
        ensure_object(full, "");
        objects_.at(full).has_value = node->value != nullptr;
    }
    for (const auto& [path, collection] : environment_.hierarchy.get_collections()) {
        ensure_object(path, "");
        objects_.at(path).hierarchy_kind = collection.kind;
    }
    if (dip::detect_artifact(artifact_) != dip::ArtifactKind::DIPH5) {
        for (const auto& [name, source] : environment_.sources.entries()) {
            if (!source.named_source) continue;
            if (objects_.find("@sources") == objects_.end()) {
                objects_.emplace("@sources", ObjectInfo{"@sources", "Sources", "", {}, false,
                                                         dip::Path::Kind::None, ObjectRole::Sources});
                objects_.at("").children.push_back("@sources");
            }
            const std::string root = name + "?";
            objects_.emplace(root, ObjectInfo{root, name, "@sources", {}, false,
                                               dip::Path::Kind::None, ObjectRole::Source, name});
            objects_.at("@sources").children.push_back(root);
            for (const auto& node : source.nodes.get_nodes()) {
                if (!node || node->path.name.empty()) continue;
                ensure_object(node->path.name, name);
                objects_.at(name + "?" + node->path.name).has_value = node->value != nullptr;
            }
            for (const auto& [path, collection] : source.hierarchy.get_collections()) {
                ensure_object(path, name);
                objects_.at(name + "?" + path).hierarchy_kind = collection.kind;
            }
        }
    }
    for (const auto& node : environment_.nodes.get_nodes()) {
        if (!node || !node->override || !node->value) continue;
        if (objects_.find("@overrides") == objects_.end()) {
            objects_.emplace("@overrides", ObjectInfo{"@overrides", "Overrides", "", {}, false,
                                                       dip::Path::Kind::None, ObjectRole::Overrides});
            objects_.at("").children.push_back("@overrides");
        }
        const std::string id = "@override?" + node->path.name;
        objects_.emplace(id, ObjectInfo{id, node->path.name, "@overrides", {}, true,
                                         dip::Path::Kind::None, ObjectRole::Override, "", node->path.name});
        objects_.at("@overrides").children.push_back(id);
    }
    const auto schemas = environment_.get_schema_manifest();
    if (!schemas.empty()) {
        objects_.emplace("@schemas", ObjectInfo{"@schemas", "Schemas", "", {}, false,
                                                 dip::Path::Kind::None, ObjectRole::Schemas});
        objects_.at("").children.push_back("@schemas");
        for (const auto& schema : schemas) {
            const std::string id = "@schema?" + schema.name;
            objects_.emplace(id, ObjectInfo{id, schema.name, "@schemas", {}, false,
                                             dip::Path::Kind::None, ObjectRole::Schema, "", schema.name});
            objects_.at("@schemas").children.push_back(id);
        }
    }
    if (!environment_.units.entries().empty()) {
        objects_.emplace("@units", ObjectInfo{"@units", "Units", "", {}, false,
                                               dip::Path::Kind::None, ObjectRole::Units});
        objects_.at("").children.push_back("@units");
        for (const auto& [name, unit] : environment_.units.entries()) {
            const std::string id = "@unit?" + name;
            objects_.emplace(id, ObjectInfo{id, name, "@units", {}, false,
                                             dip::Path::Kind::None, ObjectRole::Unit, "", name});
            objects_.at("@units").children.push_back(id);
        }
    }
    auto& root_sections = objects_.at("").children;
    root_sections.clear();
    for (const char* section : {"@dipfile", "@overrides", "@project", "@schemas", "@sources", "@units"})
        if (objects_.find(section) != objects_.end()) root_sections.emplace_back(section);
    if (objects_.find(selection_) == objects_.end()) {
        while (!selection_.empty() && objects_.find(selection_) == objects_.end())
            selection_ = parent_id(selection_);
    }
}

bool ViewerModel::reload() {
    try {
        auto fresh = dip::open_artifact(artifact_, true);
        environment_ = std::move(fresh);
        rebuild_objects();
        ++revision_;
        error_.clear();
        stale_ = false;
        history_.clear();
        history_.push_back(selection_);
        history_index_ = 0;
        return true;
    } catch (const std::exception& error) {
        error_ = error.what();
        stale_ = true;
        return false;
    }
}

bool ViewerModel::select(const std::string& path) {
    if (objects_.find(path) == objects_.end()) return false;
    if (selection_ == path) return true;
    selection_ = path;
    history_.resize(history_index_ + 1);
    history_.push_back(path);
    history_index_ = history_.size() - 1;
    return true;
}

bool ViewerModel::back() {
    if (history_index_ == 0) return false;
    selection_ = history_[--history_index_];
    return true;
}

bool ViewerModel::forward() {
    if (history_index_ + 1 >= history_.size()) return false;
    selection_ = history_[++history_index_];
    return true;
}

const ObjectInfo* ViewerModel::object(const std::string& path) const {
    const auto it = objects_.find(path);
    return it == objects_.end() ? nullptr : &it->second;
}

dip::ValueNode::PointerType ViewerModel::value_node(const ObjectInfo& object) const {
    if (!object.has_value) return nullptr;
    if (object.source_name.empty()) return environment_.get_node(object.node_path);
    for (const auto& node : environment_.sources.at(object.source_name).nodes.get_nodes())
        if (node && node->path.name == object.node_path) return node;
    return nullptr;
}

std::vector<SourceTarget> ViewerModel::source_targets(const ObjectInfo& object) const {
    if (dip::detect_artifact(artifact_) == dip::ArtifactKind::DIPH5) return {};
    if (object.role == ObjectRole::Artifact || object.role == ObjectRole::Project ||
        object.role == ObjectRole::DIPfile)
        return {{"Source", artifact_, 1, {}}};

    dip::SourceEntity entity{dip::SourceEntityKind::Path, object.node_path, object.source_name};
    if (object.role == ObjectRole::ManifestEntry && object.manifest_index) {
        entity.kind = dip::SourceEntityKind::ProjectEntry;
        entity.index = *object.manifest_index;
    } else if (object.role == ObjectRole::Source) {
        entity.kind = dip::SourceEntityKind::NamedSource;
        entity.name = object.source_name;
    } else if (object.role == ObjectRole::Schema) {
        entity.kind = dip::SourceEntityKind::Schema;
    } else if (object.role == ObjectRole::Unit) {
        entity.kind = dip::SourceEntityKind::Unit;
    } else if (object.role != ObjectRole::Path && object.role != ObjectRole::Override) {
        return {};
    }

    std::vector<SourceTarget> targets;
    for (const auto& location : dip::inspect_source_locations(environment_, entity)) {
        if (!location.source_text_available || location.source.path.empty()) continue;
        std::filesystem::path file(location.source.path);
        if (file.is_relative()) file = artifact_.parent_path() / file;
        if (file.filename() != "DIPfile" && file.extension() != ".dip" && file.extension() != ".dipl")
            continue;
        const auto duplicate = std::find_if(targets.begin(), targets.end(), [&](const auto& target) {
            return target.file == file && target.line == location.line;
        });
        if (duplicate != targets.end()) continue;
        std::string label;
        if (location.embedded_registration) label = "Embedded source registration";
        else switch (location.role) {
        case dip::SourceLocationRole::Source: label = "Source"; break;
        case dip::SourceLocationRole::Declaration: label = "Declaration"; break;
        case dip::SourceLocationRole::Modification:
            label = "Modification " + std::to_string(location.modification_index); break;
        case dip::SourceLocationRole::Override: label = "Effective override"; break;
        case dip::SourceLocationRole::Definition: label = "Definition"; break;
        case dip::SourceLocationRole::Registration: label = "Registration"; break;
        }
        targets.push_back({std::move(label), std::move(file), location.line, location.source.name});
    }
    return targets;
}

} // namespace snt::view
