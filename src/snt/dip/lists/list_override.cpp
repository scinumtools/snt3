#include <snt/dip/exceptions.h>
#include <snt/dip/lists/list_override.h>

namespace snt::dip {

    void OverrideList::append(const BaseNode::PointerType& node) {
        if (!node || node->dtype != NodeDtype::Modification || node->path.name.empty())
            throw dip::SyntaxException(
                "Invalid override entry", "Only value modifications with a target path can be overridden.",
                "Use `path = value` for each override entry.", __FILE__, __LINE__
            );
        const std::string& path = node->path.name;
        if (!entries_.emplace(path, Entry{node, false}).second)
            throw dip::SyntaxException(
                "Duplicate override",
                "The path `" + path + "` is overridden more than once.",
                "Keep only one override for each fully qualified node path.",
                __FILE__, __LINE__, node->line
            );
    }

    BaseNode::PointerType OverrideList::find(const std::string& path) const {
        const auto entry = entries_.find(path);
        return entry == entries_.end() ? nullptr : entry->second.node;
    }

    void OverrideList::consume(const std::string& path) {
        entries_.at(path).consumed = true;
    }

    std::vector<std::string> OverrideList::unresolved() const {
        std::vector<std::string> paths;
        for (const auto& [path, entry] : entries_)
            if (!entry.consumed)
                paths.push_back(path);
        return paths;
    }

} // namespace snt::dip
