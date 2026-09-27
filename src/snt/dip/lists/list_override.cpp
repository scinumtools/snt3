#include "../parsers.h"

#include <snt/dip/exceptions.h>
#include <snt/dip/lists/list_hierarchy.h>
#include <snt/dip/lists/list_override.h>

namespace snt::dip {

    void OverrideList::append(const BaseNode::ListType& nodes, size_t indent) {
        BaseNode::ListType body;
        for (const auto& node : nodes)
            if (node->dtype != NodeDtype::Empty)
                body.push_back(node);
        if (body.empty())
            throw dip::SyntaxException(
                "Empty override", "The override body has no modifications.",
                "Provide one or more value modifications.", __FILE__, __LINE__
            );
        OverrideList collected = *this;
        HierarchyList hierarchy;
        for (size_t i = 0; i < body.size(); ++i) {
            const auto& node = body.at(i);
            const bool prefix = node->dtype == NodeDtype::Group && node->dtype_raw[1].empty() &&
                                node->schemas.empty() && node->value_raw.empty();
            if ((!prefix && node->dtype != NodeDtype::Modification) || node->path.name.empty() ||
                node->path.name.find("[]") != std::string::npos)
                throw dip::SyntaxException(
                    "Invalid override entry", "Override bodies accept only value modifications and path prefixes.",
                    "Use existing paths without types, properties, schema applications, or collection appends.",
                    __FILE__, __LINE__, node->line
                );
            check_indent(i == 0 ? nullptr : body.at(i - 1), node);
            if (node->indent < indent || (i == 0 && node->indent != indent))
                throw dip::SyntaxException(
                    "Invalid override indentation", "Override children must be indented exactly one level.",
                    "Start at the body indentation and use two spaces per nested level.",
                    __FILE__, __LINE__, node->line
                );
            if (prefix && (i + 1 == body.size() || body.at(i + 1)->indent <= node->indent))
                throw dip::SyntaxException(
                    "Empty override prefix", "The path prefix `" + node->path.name + "` has no modifications.",
                    "Add nested value modifications or remove the prefix.", __FILE__, __LINE__, node->line
                );
            auto resolved = node->clone(hierarchy.get_current_path(node->indent, node->path.name));
            if (!prefix)
                collected.append(resolved);
            hierarchy.record_parent(node);
        }
        *this = std::move(collected);
    }

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
