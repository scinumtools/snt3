#include "viewer_model.h"

#include <utility>

namespace snt::view {
namespace {
std::string parent_path(const std::string& path) {
    const auto dot = path.rfind('.');
    return dot == std::string::npos ? std::string{} : path.substr(0, dot);
}
} // namespace

ViewerModel::ViewerModel(std::filesystem::path artifact)
    : artifact_(std::filesystem::absolute(std::move(artifact))) {
    environment_ = dip::open_artifact(artifact_, true);
    rebuild_objects();
    history_.push_back(selection_);
}

void ViewerModel::rebuild_objects() {
    objects_.clear();
    objects_.emplace("", ObjectInfo{"", artifact_.filename().string(), "", {}, false});
    for (const auto& node : environment_.nodes.get_nodes()) {
        if (!node) continue;
        const std::string& full = node->path.name;
        if (full.empty()) continue;
        std::size_t begin = 0;
        while (begin < full.size()) {
            const auto dot = full.find('.', begin);
            const std::string path = full.substr(0, dot == std::string::npos ? full.size() : dot);
            const std::string parent = parent_path(path);
            if (objects_.find(path) == objects_.end()) {
                objects_.emplace(path, ObjectInfo{path, path.substr(begin), parent, {}, false});
                objects_.at(parent).children.push_back(path);
            }
            if (dot == std::string::npos) break;
            begin = dot + 1;
        }
        objects_.at(full).has_value = node->value != nullptr;
    }
    if (objects_.find(selection_) == objects_.end()) {
        while (!selection_.empty() && objects_.find(selection_) == objects_.end())
            selection_ = parent_path(selection_);
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

} // namespace snt::view
