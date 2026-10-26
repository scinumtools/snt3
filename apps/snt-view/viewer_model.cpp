#include "viewer_model.h"

#include <algorithm>
#include <functional>
#include <snt/dip/artifact.h>
#include <string_view>
#include <utility>

namespace snt::view {
    namespace {
        std::string parent_path(const std::string& path) {
            if (!path.empty() && path.back() == ']') {
                const auto bracket = path.rfind('[');
                if (bracket != std::string::npos)
                    return path.substr(0, bracket);
            }
            const auto dot = path.rfind('.');
            return dot == std::string::npos ? std::string{} : path.substr(0, dot);
        }

        std::string path_label(const std::string& path, const std::string& parent) {
            if (parent.empty())
                return path;
            return path.substr(parent.size() + (path[parent.size()] == '.' ? 1 : 0));
        }

        std::string parent_id(const std::string& id) {
            if (id == "@project" || id == "@dipfile" || id == "@local" || id == "@blocks" || id == "@sources" ||
                id == "@raw_sources" || id == "@overrides" || id == "@schemas" || id == "@units" ||
                id == "@named-nodes")
                return "";
            constexpr std::string_view named_nodes_prefix = "@named-nodes?";
            if (id.compare(0, named_nodes_prefix.size(), named_nodes_prefix) == 0) {
                const auto separator = id.find('?', named_nodes_prefix.size());
                if (separator == std::string::npos)
                    return "@named-nodes";
                const std::string group = id.substr(0, separator);
                const std::string node_path = id.substr(separator + 1);
                const std::string parent = parent_path(node_path);
                return parent.empty() ? group : group + "?" + parent;
            }
            if (id == "@dipfile-source")
                return "@local";
            if (id.compare(0, 9, "@dipfile?") == 0) {
                const auto next = id.find('?', 9);
                return next == std::string::npos ? "@dipfile" : id.substr(0, next);
            }
            if (id.compare(0, 10, "@override?") == 0)
                return "@overrides";
            if (id.compare(0, 6, "@code?") == 0)
                return "@local";
            if (id.compare(0, 7, "@block?") == 0)
                return "@blocks";
            if (id.compare(0, 8, "@schema?") == 0)
                return "@schemas";
            if (id.compare(0, 6, "@unit?") == 0)
                return "@units";
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
        environment_ = dip::open_artifact(artifact_, true, true);
        if (dip::detect_artifact(artifact_) == dip::ArtifactKind::Project) {
            try {
                edit_session_ = std::make_unique<dip::EditSession>(artifact_);
            } catch (const std::exception& error) {
                error_ = std::string("Project opened read-only: ") + error.what();
            }
        }
        rebuild_objects();
        history_.push_back(selection_);
    }

    std::string ViewerModel::display_file_path(const std::string& path) const {
        if (path.empty())
            return {};
        const std::filesystem::path file(path);
        if (!file.is_absolute())
            return file.generic_string();
        const auto relative = file.lexically_relative(artifact_.parent_path());
        return relative.empty() ? file.filename().generic_string() : relative.generic_string();
    }

    void ViewerModel::rebuild_objects() {
        const dip::Inspector view{environment_};
        objects_.clear();
        objects_.emplace(
            "",
            ObjectInfo{"", artifact_.filename().string(), "", {}, false, dip::Path::Kind::None, ObjectRole::Artifact}
        );
        objects_.emplace(
            "@project",
            ObjectInfo{"@project", "Resolved nodes", "", {}, false, dip::Path::Kind::None, ObjectRole::Project}
        );
        objects_.at("").children.push_back("@project");
        if (dip::detect_artifact(artifact_) == dip::ArtifactKind::Project) {
            objects_.emplace(
                "@dipfile", ObjectInfo{"@dipfile", "DIPfile", "", {}, false, dip::Path::Kind::None, ObjectRole::DIPfile}
            );
            objects_.emplace(
                "@local",
                ObjectInfo{"@local", "Local sources", "", {}, false, dip::Path::Kind::None, ObjectRole::LocalSources}
            );
            objects_.at("").children.push_back("@local");
            ObjectInfo manifest_source{
                "@dipfile-source", "DIPfile", "@local", {}, false, dip::Path::Kind::None, ObjectRole::DIPfileSource
            };
            manifest_source.source_kind = "manifest";
            objects_.emplace("@dipfile-source", std::move(manifest_source));
            objects_.at("@local").children.push_back("@dipfile-source");
            for (std::size_t index = 0; index < environment_.project_entries().size(); ++index) {
                const auto& entry = environment_.project_entries()[index];
                const char* category = nullptr;
                switch (entry.kind) {
                case dip::ProjectEntry::Kind::Unit:
                    category = "Units";
                    break;
                case dip::ProjectEntry::Kind::Source:
                    category = "Sources";
                    break;
                case dip::ProjectEntry::Kind::Schema:
                    category = "Schemas";
                    break;
                case dip::ProjectEntry::Kind::Code:
                    category = "Code";
                    break;
                case dip::ProjectEntry::Kind::Override:
                    category = "Overrides";
                    break;
                }
                const std::string category_id = std::string("@dipfile?") + category;
                if (objects_.find(category_id) == objects_.end()) {
                    objects_.emplace(
                        category_id,
                        ObjectInfo{
                            category_id,
                            category,
                            "@dipfile",
                            {},
                            false,
                            dip::Path::Kind::None,
                            ObjectRole::ManifestCategory
                        }
                    );
                    objects_.at("@dipfile").children.push_back(category_id);
                }
                const std::string id = category_id + "?" + std::to_string(index);
                const std::string label =
                    entry.name.empty() ? (entry.resolved_path.empty() ? "Inline text" : entry.value) : entry.name;
                ObjectInfo item{id, label, category_id, {}, false, dip::Path::Kind::None, ObjectRole::ManifestEntry};
                item.manifest_index = index;
                objects_.emplace(id, std::move(item));
                objects_.at(category_id).children.push_back(id);
            }
        }
        std::function<void(const std::string&, const std::string&)> ensure_object = [&](const std::string& path,
                                                                                        const std::string& source) {
            if (path.empty())
                return;
            const std::string id = source.empty() ? path : source + "?" + path;
            if (objects_.find(id) != objects_.end())
                return;
            const std::string parent = parent_path(path);
            if (!parent.empty())
                ensure_object(parent, source);
            const std::string parent_id =
                source.empty() ? (parent.empty() ? "@project" : parent) : source + "?" + parent;
            objects_.emplace(
                id,
                ObjectInfo{
                    id,
                    path_label(path, parent),
                    parent_id,
                    {},
                    false,
                    dip::Path::Kind::None,
                    ObjectRole::Path,
                    source,
                    path
                }
            );
            objects_.at(parent_id).children.push_back(id);
        };
        std::function<void(const std::string&, const std::string&)> ensure_named_source_object =
            [&](const std::string& path, const std::string& source) {
                if (path.empty())
                    return;
                const std::string group = "@named-nodes?" + source;
                const std::string id = group + "?" + path;
                if (objects_.find(id) != objects_.end())
                    return;
                const std::string parent = parent_path(path);
                if (!parent.empty())
                    ensure_named_source_object(parent, source);
                const std::string parent_entry = parent.empty() ? group : group + "?" + parent;
                objects_.emplace(
                    id,
                    ObjectInfo{
                        id,
                        path_label(path, parent),
                        parent_entry,
                        {},
                        false,
                        dip::Path::Kind::None,
                        ObjectRole::Path,
                        source,
                        path
                    }
                );
                objects_.at(parent_entry).children.push_back(id);
            };
        for (const auto& node : environment_.nodes.get_nodes()) {
            if (!node)
                continue;
            const std::string& full = node->path.name;
            if (full.empty())
                continue;
            ensure_object(full, "");
            objects_.at(full).has_value = node->value != nullptr;
            if (!node->table_path.empty()) {
                ensure_object(node->table_path, "");
                objects_.at(node->table_path).has_table = true;
            }
        }
        for (const auto& [path, collection] : environment_.hierarchy.get_collections()) {
            ensure_object(path, "");
            objects_.at(path).hierarchy_kind = collection.kind;
        }
        if (dip::detect_artifact(artifact_) != dip::ArtifactKind::DIPH5) {
            if (std::any_of(environment_.sources.entries().begin(), environment_.sources.entries().end(), [](const auto& entry) {
                    return entry.second.named_source;
                }) && objects_.find("@local") == objects_.end()) {
                objects_.emplace(
                    "@local",
                    ObjectInfo{"@local", "Local sources", "", {}, false, dip::Path::Kind::None, ObjectRole::LocalSources}
                );
                objects_.at("").children.push_back("@local");
            }
            for (const auto& [name, source] : environment_.sources.entries()) {
                if (!source.named_source)
                    continue;
                const std::string category = source.raw_text ? "@raw_sources" : "@sources";
                if (objects_.find(category) == objects_.end()) {
                    objects_.emplace(
                        category,
                        ObjectInfo{
                            category,
                            source.raw_text ? "Raw named sources" : "Named sources",
                            "",
                            {},
                            false,
                            dip::Path::Kind::None,
                            source.raw_text ? ObjectRole::RawSources : ObjectRole::Sources
                        }
                    );
                    objects_.at("").children.push_back(category);
                }
                const std::string root = name + "?";
                std::string kind = source.table_text ? "table" : "text";
                if (!source.table_text) {
                    for (const auto& node : environment_.nodes.get_nodes()) {
                        if (node && node->value_origin == dip::ValueOrigin::ReferenceRaw && !node->value_raw.empty() &&
                            node->value_raw.front() == name &&
                            (node->dtype == dip::NodeDtype::Boolean || node->dtype == dip::NodeDtype::Integer ||
                             node->dtype == dip::NodeDtype::Float) &&
                            !node->dimension.empty()) {
                            kind = "array";
                            break;
                        }
                    }
                }
                const std::string label = name;
                objects_.emplace(
                    root, ObjectInfo{root, label, category, {}, false, dip::Path::Kind::None, ObjectRole::Source, name}
                );
                objects_.at(root).source_kind = source.raw_text ? kind : "dipl";
                objects_.at(category).children.push_back(root);
                bool has_source_nodes = !source.nodes.get_nodes().empty() || !source.hierarchy.get_collections().empty();
                if (has_source_nodes && objects_.find("@named-nodes") == objects_.end()) {
                    objects_.emplace(
                        "@named-nodes",
                        ObjectInfo{
                            "@named-nodes",
                            "Named source nodes",
                            "",
                            {},
                            false,
                            dip::Path::Kind::None,
                            ObjectRole::NamedSourceNodes
                        }
                    );
                }
                if (has_source_nodes) {
                    const std::string node_group = "@named-nodes?" + name;
                    objects_.emplace(
                        node_group,
                        ObjectInfo{
                            node_group,
                            label,
                            "@named-nodes",
                            {},
                            false,
                            dip::Path::Kind::None,
                            ObjectRole::NamedSourceNodeGroup,
                            name
                        }
                    );
                    objects_.at("@named-nodes").children.push_back(node_group);
                }
                for (const auto& node : source.nodes.get_nodes()) {
                    if (!node || node->path.name.empty())
                        continue;
                    ensure_named_source_object(node->path.name, name);
                    objects_.at("@named-nodes?" + name + "?" + node->path.name).has_value = node->value != nullptr;
                }
                for (const auto& [path, collection] : source.hierarchy.get_collections()) {
                    ensure_named_source_object(path, name);
                    objects_.at("@named-nodes?" + name + "?" + path).hierarchy_kind = collection.kind;
                }
            }
        }
        for (std::size_t index = 0; index < environment_.project_entries().size(); ++index) {
            const auto& entry = environment_.project_entries()[index];
            const bool is_code = entry.kind == dip::ProjectEntry::Kind::Code;
            const bool is_override = entry.kind == dip::ProjectEntry::Kind::Override;
            if (!is_code && !is_override)
                continue;
            if (objects_.find("@local") == objects_.end()) {
                objects_.emplace(
                    "@local",
                    ObjectInfo{
                        "@local", "Local sources", "", {}, false, dip::Path::Kind::None, ObjectRole::LocalSources
                    }
                );
                objects_.at("").children.push_back("@local");
            }
            const std::string id = "@code?" + std::to_string(index);
            std::string label = entry.resolved_path.empty()
                                    ? "Inline code (DIPfile:" + std::to_string(entry.line) + ")"
                                    : display_file_path(entry.resolved_path);
            ObjectInfo item{id, label, "@local", {}, false, dip::Path::Kind::None, ObjectRole::CodeSource};
            item.manifest_index = index;
            item.source_kind = is_override ? "override" : "code";
            objects_.emplace(id, std::move(item));
            objects_.at("@local").children.push_back(id);
        }
        if (!view.block_inputs().empty()) {
            objects_.emplace(
                "@blocks",
                ObjectInfo{
                    "@blocks", "Block value sources", "", {}, false, dip::Path::Kind::None, ObjectRole::BlockSources
                }
            );
        }
        std::vector<std::string> block_paths;
        block_paths.reserve(view.block_inputs().size());
        for (const auto& entry : view.block_inputs())
            block_paths.push_back(entry.first);
        std::sort(block_paths.begin(), block_paths.end(), [&](const std::string& lhs, const std::string& rhs) {
            const auto left_kind = view.block_inputs().at(lhs).kind;
            const auto right_kind = view.block_inputs().at(rhs).kind;
            if (left_kind != right_kind)
                return left_kind == dip::BlockInput::Kind::Array;
            return lhs < rhs;
        });
        for (const auto& path : block_paths) {
            const auto& input = view.block_inputs().at(path);
            const std::string id = "@block?" + path;
            const std::string label = path;
            objects_.emplace(
                id,
                ObjectInfo{id, label, "@blocks", {}, false, dip::Path::Kind::None, ObjectRole::BlockSource, "", path}
            );
            objects_.at(id).source_kind = input.kind == dip::BlockInput::Kind::Table ? "table" : "array";
            objects_.at("@blocks").children.push_back(id);
        }
        for (const auto& node : environment_.nodes.get_nodes()) {
            if (!node || !node->override || !node->value)
                continue;
            if (objects_.find("@overrides") == objects_.end()) {
                objects_.emplace(
                    "@overrides",
                    ObjectInfo{
                        "@overrides", "Overridden nodes", "", {}, false, dip::Path::Kind::None, ObjectRole::Overrides
                    }
                );
                objects_.at("").children.push_back("@overrides");
            }
            const std::string id = "@override?" + node->path.name;
            objects_.emplace(
                id,
                ObjectInfo{
                    id,
                    node->path.name,
                    "@overrides",
                    {},
                    true,
                    dip::Path::Kind::None,
                    ObjectRole::Override,
                    "",
                    node->path.name
                }
            );
            objects_.at("@overrides").children.push_back(id);
        }
        const auto schemas = environment_.get_schema_manifest();
        if (!schemas.empty()) {
            objects_.emplace(
                "@schemas", ObjectInfo{"@schemas", "Schemas", "", {}, false, dip::Path::Kind::None, ObjectRole::Schemas}
            );
            objects_.at("").children.push_back("@schemas");
            for (const auto& schema : schemas) {
                const std::string id = "@schema?" + schema.name;
                objects_.emplace(
                    id,
                    ObjectInfo{
                        id,
                        schema.name,
                        "@schemas",
                        {},
                        false,
                        dip::Path::Kind::None,
                        ObjectRole::Schema,
                        "",
                        schema.name
                    }
                );
                objects_.at("@schemas").children.push_back(id);
            }
        }
        if (!environment_.units.entries().empty()) {
            objects_.emplace(
                "@units", ObjectInfo{"@units", "Units", "", {}, false, dip::Path::Kind::None, ObjectRole::Units}
            );
            objects_.at("").children.push_back("@units");
            for (const auto& [name, unit] : environment_.units.entries()) {
                const std::string id = "@unit?" + name;
                objects_.emplace(
                    id, ObjectInfo{id, name, "@units", {}, false, dip::Path::Kind::None, ObjectRole::Unit, "", name}
                );
                objects_.at("@units").children.push_back(id);
            }
        }
        auto& root_sections = objects_.at("").children;
        root_sections.clear();
        for (const char* section :
             {"@dipfile",
              "@overrides",
              "@project",
              "@named-nodes",
              "@schemas",
              "@units",
              "@local",
              "@sources",
              "@blocks",
              "@raw_sources"})
            if (objects_.find(section) != objects_.end())
                root_sections.emplace_back(section);
        if (objects_.find(selection_) == objects_.end()) {
            while (!selection_.empty() && objects_.find(selection_) == objects_.end())
                selection_ = parent_id(selection_);
        }
    }

    bool ViewerModel::reload() {
        try {
            auto fresh = dip::open_artifact(artifact_, true, true);
            std::unique_ptr<dip::EditSession> fresh_session;
            std::string edit_error;
            if (dip::detect_artifact(artifact_) == dip::ArtifactKind::Project) {
                try {
                    fresh_session = std::make_unique<dip::EditSession>(artifact_);
                } catch (const std::exception& error) {
                    edit_error = std::string("Project opened read-only: ") + error.what();
                }
            }
            environment_ = std::move(fresh);
            edit_session_ = std::move(fresh_session);
            rebuild_objects();
            ++revision_;
            error_ = std::move(edit_error);
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
        if (objects_.find(path) == objects_.end())
            return false;
        if (selection_ == path)
            return true;
        selection_ = path;
        history_.resize(history_index_ + 1);
        history_.push_back(path);
        history_index_ = history_.size() - 1;
        return true;
    }

    bool ViewerModel::back() {
        if (history_index_ == 0)
            return false;
        selection_ = history_[--history_index_];
        return true;
    }

    bool ViewerModel::forward() {
        if (history_index_ + 1 >= history_.size())
            return false;
        selection_ = history_[++history_index_];
        return true;
    }

    const ObjectInfo* ViewerModel::object(const std::string& path) const {
        const auto it = objects_.find(path);
        return it == objects_.end() ? nullptr : &it->second;
    }

    dip::ValueNode::PointerType ViewerModel::value_node(const ObjectInfo& object) const {
        if (!object.has_value)
            return nullptr;
        if (object.source_name.empty())
            return environment_.get_node(object.node_path);
        for (const auto& node : environment_.sources.at(object.source_name).nodes.get_nodes())
            if (node && node->path.name == object.node_path)
                return node;
        return nullptr;
    }

    std::string ViewerModel::data_path(const ObjectInfo& object) const {
        if (object.role != ObjectRole::Source)
            return object.source_name.empty() ? object.node_path : object.path;
        const auto source = environment_.sources.entries().find(object.source_name);
        if (source == environment_.sources.entries().end() || !source->second.raw_text)
            return {};
        if (source->second.table_text)
            return source->second.table_path;
        for (const auto& node : environment_.nodes.get_nodes())
            if (node && node->value_origin == dip::ValueOrigin::ReferenceRaw && !node->value_raw.empty() &&
                node->value_raw.front() == object.source_name && node->value &&
                (node->dtype == dip::NodeDtype::Table ||
                 ((node->dtype == dip::NodeDtype::Boolean || node->dtype == dip::NodeDtype::Integer ||
                   node->dtype == dip::NodeDtype::Float) &&
                  !node->dimension.empty())))
                return node->path.name;
        return {};
    }

    std::vector<SourceTarget> ViewerModel::source_targets(const ObjectInfo& object) const {
        const dip::Inspector view{environment_};
        if (dip::detect_artifact(artifact_) == dip::ArtifactKind::DIPH5)
            return {};
        if (object.role == ObjectRole::DIPfileSource)
            return {{"DIPfile", artifact_, 1, {}}};
        if (object.role == ObjectRole::Artifact || object.role == ObjectRole::Project ||
            object.role == ObjectRole::DIPfile)
            return {{"Source", artifact_, 1, {}}};
        if (object.role == ObjectRole::CodeSource && object.manifest_index) {
            const auto& entry = environment_.project_entries().at(*object.manifest_index);
            return {
                {"Source",
                 entry.resolved_path.empty() ? artifact_ : std::filesystem::path(entry.resolved_path),
                 entry.resolved_path.empty() ? entry.line : 1,
                 {}}
            };
        }
        if (object.role == ObjectRole::BlockSource) {
            const auto& input = view.block_inputs().at(object.node_path);
            const auto& source = environment_.sources.at(input.source_name);
            std::filesystem::path file = source.path;
            std::size_t line = input.source_line;
            if (source.embedded) {
                file = environment_.sources.at(source.parent.name).path;
                line = source.parent.line_number;
            }
            if (file.is_relative())
                file = std::filesystem::absolute(file);
            const auto format = input.kind == dip::BlockInput::Kind::Table ? SourceFormat::Table : SourceFormat::Plain;
            return {{"Block value", std::move(file), line, {}, format, input.path}};
        }

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
        SourceFormat format = SourceFormat::DIPL;
        if (object.role == ObjectRole::Source) {
            const auto& source = environment_.sources.at(object.source_name);
            if (source.table_text)
                format = SourceFormat::Table;
            else if (source.raw_text)
                format = SourceFormat::Plain;
        }
        for (const auto& location : view.source_locations(entity)) {
            if (!location.source_text_available || location.source.path.empty())
                continue;
            std::filesystem::path file(location.source.path);
            if (file.is_relative())
                file = std::filesystem::absolute(file);
            if (format == SourceFormat::DIPL && file.filename() != "DIPfile" && file.extension() != ".dip" &&
                file.extension() != ".dipl")
                continue;
            const auto duplicate = std::find_if(targets.begin(), targets.end(), [&](const auto& target) {
                return target.file == file && target.line == location.line;
            });
            if (duplicate != targets.end())
                continue;
            std::string label;
            if (location.embedded_registration)
                label = "Embedded source registration";
            else
                switch (location.role) {
                case dip::SourceLocationRole::Source:
                    label = "Source";
                    break;
                case dip::SourceLocationRole::Declaration:
                    label = "Declaration";
                    break;
                case dip::SourceLocationRole::Modification:
                    label = "Modification " + std::to_string(location.modification_index);
                    break;
                case dip::SourceLocationRole::Override:
                    label = "Effective override";
                    break;
                case dip::SourceLocationRole::Definition:
                    label = "Definition";
                    break;
                case dip::SourceLocationRole::Registration:
                    label = "Registration";
                    break;
                }
            targets.push_back({std::move(label), std::move(file), location.line, location.source.name, format});
        }
        return targets;
    }

    std::string ViewerModel::source_object_path(const SourceTarget& target) const {
        if (target.block_path) {
            const std::string path = "@block?" + *target.block_path;
            if (object(path))
                return path;
        }
        if (!target.source_name.empty()) {
            const std::string path = target.source_name + "?";
            if (const auto* entry = object(path); entry && entry->role == ObjectRole::Source)
                return path;
        }
        const auto normalized = [](const std::filesystem::path& path) {
            std::error_code error;
            const auto canonical = std::filesystem::weakly_canonical(path, error);
            return error ? std::filesystem::absolute(path).lexically_normal() : canonical;
        };
        const auto target_file = normalized(target.file);
        if (object("@dipfile-source") && target_file == normalized(artifact_))
            return "@dipfile-source";
        for (const auto& [path, entry] : objects_) {
            if (entry.role != ObjectRole::CodeSource || !entry.manifest_index)
                continue;
            const auto& project_entry = environment_.project_entries().at(*entry.manifest_index);
            if (!project_entry.resolved_path.empty() && normalized(project_entry.resolved_path) == target_file)
                return path;
        }
        return {};
    }

    std::string ViewerModel::named_source_node_path(const std::string& source, const std::string& node_path) const {
        const std::string path = "@named-nodes?" + source + "?" + node_path;
        return object(path) ? path : std::string{};
    }

    std::optional<std::string> ViewerModel::editable_document_path(const std::filesystem::path& file) const {
        if (!edit_session_ || file.empty())
            return std::nullopt;
        try {
            const auto root = std::filesystem::canonical(artifact_).parent_path();
            const auto resolved = std::filesystem::canonical(file);
            const auto relative = resolved.lexically_relative(root);
            if (relative.empty() || relative.is_absolute() || *relative.begin() == "..")
                return std::nullopt;
            const std::string path = relative.generic_string();
            return edit_session_->contains_document(path) ? std::optional<std::string>(path) : std::nullopt;
        } catch (const std::filesystem::filesystem_error&) {
            return std::nullopt;
        }
    }

    const std::string& ViewerModel::document_text(const std::string& path) const {
        if (!edit_session_)
            throw std::logic_error("No editable project session is active.");
        return edit_session_->document_text(path);
    }

    dip::PreviewResult ViewerModel::preview_document(const std::string& path, const std::string& text) const {
        if (!edit_session_)
            throw std::logic_error("No editable project session is active.");
        return edit_session_->preview_document(path, text);
    }

    dip::PreviewResult ViewerModel::preview_block(const std::string& path, const std::string& text) const {
        if (!edit_session_)
            throw std::logic_error("No editable project session is active.");
        return edit_session_->preview_block(path, text);
    }

    bool ViewerModel::save_document(const std::string& path, const std::string& text) {
        if (!edit_session_) {
            error_ = "No editable project session is active.";
            return false;
        }
        try {
            edit_session_->save_document(path, text);
            reload(); // Preserve the last valid environment if the saved candidate is not valid yet.
            return true;
        } catch (const std::exception& error) {
            error_ = error.what();
            return false;
        }
    }

    bool ViewerModel::save_block(const std::string& path, const std::string& text) {
        if (!edit_session_) {
            error_ = "No editable project session is active.";
            return false;
        }
        try {
            edit_session_->save_block(path, text);
            reload();
            return true;
        } catch (const std::exception& error) {
            error_ = error.what();
            return false;
        }
    }

    bool ViewerModel::save_as(
        const std::filesystem::path& destination_directory,
        const std::optional<std::string>& edited_path,
        const std::string& edited_text
    ) {
        if (!edit_session_) {
            error_ = "Save As is available only for file-backed DIP projects.";
            return false;
        }
        try {
            if (!edit_session_->save_as(destination_directory, edited_path, edited_text)) {
                error_ = "Cannot create the copy until the candidate project evaluates successfully. Check and fix its errors first.";
                return false;
            }
            artifact_ = std::filesystem::absolute(destination_directory / edit_session_->bundle().entry);
            input_path_ = artifact_.string();
            if (!reload())
                return false;
            return true;
        } catch (const std::exception& error) {
            error_ = error.what();
            return false;
        }
    }

} // namespace snt::view
