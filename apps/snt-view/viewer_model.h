#ifndef SNT_VIEW_VIEWER_MODEL_H
#define SNT_VIEW_VIEWER_MODEL_H

#include <snt/dip/inspection.h>

#include <filesystem>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace snt::view {

struct ObjectInfo {
    std::string path;
    std::string label;
    std::string parent;
    std::vector<std::string> children;
    bool has_value = false;
};

class ViewerModel {
public:
    explicit ViewerModel(std::filesystem::path artifact);

    bool reload();
    bool select(const std::string& path);
    bool back();
    bool forward();
    const ObjectInfo* object(const std::string& path) const;
    const dip::Environment& environment() const { return environment_; }
    const std::filesystem::path& artifact() const { return artifact_; }
    const std::string& selection() const { return selection_; }
    const std::string& search() const { return search_; }
    void set_search(std::string query) { search_ = std::move(query); }
    const std::string& error() const { return error_; }
    bool stale() const { return stale_; }
    std::size_t revision() const { return revision_; }

private:
    void rebuild_objects();
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
