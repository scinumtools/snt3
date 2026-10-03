#ifndef SNT_VIEW_VIEWER_MODEL_H
#define SNT_VIEW_VIEWER_MODEL_H

#include <snt/dip/inspect/inspection.h>
#include "source_format.h"

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace snt::view {

enum class ObjectRole {
    Artifact, Project, DIPfile, ManifestCategory, ManifestEntry,
    LocalSources, CodeSource, BlockSources, BlockSource, Sources, RawSources, Source,
    Overrides, Override, Schemas, Schema, Units, Unit, Path
};

struct ObjectInfo {
    std::string path;
    std::string label;
    std::string parent;
    std::vector<std::string> children;
    bool has_value = false;
    dip::Path::Kind hierarchy_kind = dip::Path::Kind::None;
    ObjectRole role = ObjectRole::Path;
    std::string source_name;
    std::string node_path;
    std::optional<std::size_t> manifest_index;
};

struct SourceTarget {
    std::string label;
    std::filesystem::path file;
    std::size_t line = 0;
    std::string source_name;
    SourceFormat format = SourceFormat::DIPL;
    std::optional<std::string> block_path;
};

class ViewerModel {
public:
    explicit ViewerModel(std::filesystem::path artifact);

    bool reload();
    bool select(const std::string& path);
    bool back();
    bool forward();
    bool can_back() const { return history_index_ > 0; }
    bool can_forward() const { return history_index_ + 1 < history_.size(); }
    const ObjectInfo* object(const std::string& path) const;
    dip::ValueNode::PointerType value_node(const ObjectInfo& object) const;
    std::vector<SourceTarget> source_targets(const ObjectInfo& object) const;
    const dip::Environment& environment() const { return environment_; }
    const std::filesystem::path& artifact() const { return artifact_; }
    const std::string& input_path() const { return input_path_; }
    std::string display_file_path(const std::string& path) const;
    const std::string& selection() const { return selection_; }
    const std::string& search() const { return search_; }
    void set_search(std::string query) { search_ = std::move(query); }
    const std::string& error() const { return error_; }
    bool stale() const { return stale_; }
    std::size_t revision() const { return revision_; }

private:
    void rebuild_objects();
    std::string input_path_;
    std::filesystem::path artifact_;
    dip::Environment environment_;
    std::map<std::string, ObjectInfo> objects_;
    std::string selection_;
    std::string search_;
    std::string error_;
    bool stale_ = false;
    std::size_t revision_ = 0;
    std::vector<std::string> history_;
    std::size_t history_index_ = 0;
};

} // namespace snt::view

#endif
