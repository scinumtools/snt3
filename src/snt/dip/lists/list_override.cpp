#include "../nodes/node_group.h"
#include "../parsers.h"

#include <algorithm>
#include <snt/dip/exceptions.h>
#include <snt/dip/lists/list_hierarchy.h>
#include <snt/dip/lists/list_override.h>

namespace snt::dip {

    OverrideItemTarget classify_override_item(const HierarchyList& hierarchy, const std::string& requested) {
        OverrideItemTarget result;
        const size_t selector = requested.rfind('[');
        if (requested.empty() || requested.back() != ']' || selector == std::string::npos)
            return result;
        if (requested.find("[]") != std::string::npos && selector != requested.find("[]")) {
            result.status = OverrideItemTarget::Status::UnresolvedParent;
            return result;
        }
        const std::string collection_path = requested.substr(0, selector);
        const std::string key = requested.substr(selector + 1, requested.size() - selector - 2);
        if (!hierarchy.has_collection(collection_path)) {
            result.status = OverrideItemTarget::Status::UnknownCollection;
            return result;
        }
        const Collection& collection = hierarchy.get_collection(collection_path);
        result.schemas = collection.schemas;
        const bool existing_map = collection.kind == Path::Kind::Map &&
            std::find(collection.items.begin(), collection.items.end(), key) != collection.items.end();
        const bool existing_list = collection.kind == Path::Kind::List && !key.empty() &&
            std::find(collection.items.begin(), collection.items.end(), key) != collection.items.end();
        if ((collection.kind == Path::Kind::List && !key.empty() && !existing_list) ||
            (collection.kind == Path::Kind::Map && key.empty()) ||
            (collection.kind != Path::Kind::List && collection.kind != Path::Kind::Map))
            return result;
        if (!existing_map && !existing_list && collection.schemas.empty()) {
            result.status = OverrideItemTarget::Status::SchemaRequired;
            return result;
        }
        result.status = existing_map || existing_list ? OverrideItemTarget::Status::Existing
                                                      : OverrideItemTarget::Status::Creatable;
        result.resolved_path = collection.kind == Path::Kind::List && !existing_list
            ? collection_path + "[" + std::to_string(collection.items.size()) + "]" : requested;
        return result;
    }

    void OverrideList::append(const BaseNode::ListType& nodes, size_t indent) {
        BaseNode::ListType body;
        for (const auto& node : nodes)
            if (node->dtype != NodeDtype::Empty)
                body.push_back(node);
        if (body.empty())
            throw dip::SyntaxException(
                "Empty override", "The override body has no entries.",
                "Provide one or more value modifications or collection items.", __FILE__, __LINE__
            );
        OverrideList collected = *this;
        HierarchyList hierarchy;
        std::vector<size_t> active_additions;
        for (size_t i = 0; i < body.size(); ++i) {
            const auto& node = body.at(i);
            while (!active_additions.empty() &&
                   node->indent <= collected.additions_.at(active_additions.back()).node->indent)
                active_additions.pop_back();
            const bool group = node->dtype == NodeDtype::Group && node->dtype_raw[1].empty() &&
                               node->schemas.empty() && node->value_raw.empty();
            const bool item = group && !node->path.name.empty() && node->path.name.back() == ']';
            if ((!group && node->dtype != NodeDtype::Modification) || node->path.name.empty() ||
                (!item && active_additions.empty() && node->path.name.find("[]") != std::string::npos))
                throw dip::SyntaxException(
                    "Invalid override entry", "Override bodies accept value modifications and untyped path groups.",
                    "End a group path with [key] or [] to create a schema-backed collection item.",
                    __FILE__, __LINE__, node->line
                );
            check_indent(i == 0 ? nullptr : body.at(i - 1), node);
            if (node->indent < indent || (i == 0 && node->indent != indent))
                throw dip::SyntaxException(
                    "Invalid override indentation", "Override children must be indented exactly one level.",
                    "Start at the body indentation and use two spaces per nested level.",
                    __FILE__, __LINE__, node->line
                );
            if (group && !item && (i + 1 == body.size() || body.at(i + 1)->indent <= node->indent))
                throw dip::SyntaxException(
                    "Empty override prefix", "The path prefix `" + node->path.name + "` has no entries.",
                    "Add nested value modifications or collection items, or remove the prefix.",
                    __FILE__, __LINE__, node->line
                );
            auto resolved = node->clone(hierarchy.get_current_path(node->indent, node->path.name));
            if (item) {
                if (!active_additions.empty())
                    collected.additions_.at(active_additions.back()).has_children = true;
                collected.additions_.push_back(
                    {resolved, active_additions.empty() ? std::nullopt : std::optional<size_t>(active_additions.back()),
                     {}, {}, false}
                );
                active_additions.push_back(collected.additions_.size() - 1);
            } else if (node->dtype == NodeDtype::Modification && !active_additions.empty()) {
                auto& owner = collected.additions_.at(active_additions.back());
                const std::string prefix_path = owner.node->path.name + ".";
                if (resolved->path.name.compare(0, prefix_path.size(), prefix_path) != 0)
                    throw dip::SyntaxException(
                        "Invalid item child", "The child modification is outside its collection item.",
                        "Indent the child under the item being added.", __FILE__, __LINE__, node->line
                    );
                owner.has_children = true;
                if (resolved->path.name.find("[]") == std::string::npos)
                    collected.append(resolved);
                else {
                    for (const auto& existing : owner.modifications)
                        if (existing->path.name == resolved->path.name)
                            throw dip::SyntaxException(
                                "Duplicate override", "The path `" + resolved->path.name + "` is overridden more than once.",
                                "Keep only one override for each value.", __FILE__, __LINE__, node->line
                            );
                    owner.modifications.push_back(resolved);
                }
            } else if (!group)
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

    size_t OverrideList::item_group_count() const { return additions_.size(); }

    std::string OverrideList::addition_path(size_t index) const {
        const auto& addition = additions_.at(index);
        std::string path = addition.node->path.name;
        if (addition.parent) {
            const auto& parent = additions_.at(*addition.parent);
            const std::string& prefix = parent.node->path.name;
            if (path.compare(0, prefix.size() + 1, prefix + ".") != 0 || parent.resolved_path.empty())
                throw dip::SyntaxException(
                    "Invalid nested item", "The nested collection path does not belong to its parent item.",
                    "Declare the nested collection under the parent item.",
                    __FILE__, __LINE__, addition.node->line
                );
            path.replace(0, prefix.size(), parent.resolved_path);
        }
        return path;
    }

    const Line& OverrideList::addition_line(size_t index) const { return additions_.at(index).node->line; }

    void OverrideList::activate_addition(size_t index, const std::string& resolved_path) {
        auto& addition = additions_.at(index);
        const std::string& prefix = addition.node->path.name;
        OverrideList collected = *this;
        for (const auto& modification : addition.modifications) {
            std::string path = resolved_path + modification->path.name.substr(prefix.size());
            if (path.find("[]") != std::string::npos)
                throw dip::SyntaxException(
                    "Unresolved collection selector", "A value modification contains an uncreated list item.",
                    "Use a nested item group for each new list item.", __FILE__, __LINE__, modification->line
                );
            collected.append(modification->clone(Path(path)));
        }
        collected.additions_.at(index).modifications.clear();
        collected.additions_.at(index).resolved_path = resolved_path;
        *this = std::move(collected);
    }

    void OverrideList::activate_concrete_descendants(size_t index) {
        for (size_t child = index + 1; child < additions_.size(); ++child) {
            const auto& addition = additions_.at(child);
            if (!addition.parent || additions_.at(*addition.parent).resolved_path.empty() ||
                !addition.resolved_path.empty())
                continue;
            const std::string path = addition_path(child);
            if (path.find("[]") == std::string::npos)
                activate_addition(child, path);
        }
    }

    BaseNode::PointerType OverrideList::materialize_item_group(size_t index, const HierarchyList& hierarchy) {
        const std::string requested = addition_path(index);
        const auto target = classify_override_item(hierarchy, requested);
        const Line& line = addition_line(index);
        if (target.status == OverrideItemTarget::Status::UnresolvedParent)
            throw dip::SyntaxException(
                "Unresolved collection selector", "An enclosing list item has not been created.",
                "Nest collection items under the list item they extend.", __FILE__, __LINE__, line
            );
        if (target.status == OverrideItemTarget::Status::UnknownCollection)
            throw dip::EnvironmentException(
                "Unknown collection", "The collection `" + requested.substr(0, requested.rfind('[')) + "` was not found.",
                "Declare the collection outside $override before adding an item.", __FILE__, __LINE__, line
            );
        if (target.status == OverrideItemTarget::Status::InvalidSelector)
            throw dip::EnvironmentException(
                "Invalid collection item", "The selector does not match an existing item or collection kind.",
                "Use [key] for maps, [] to append to lists, or an existing list index.", __FILE__, __LINE__, line
            );
        if (target.status == OverrideItemTarget::Status::SchemaRequired)
            throw dip::EnvironmentException(
                "Invalid collection item", "New override items require a schema-backed collection.",
                "Declare an item schema on the map or list.", __FILE__, __LINE__, line
            );
        activate_addition(index, target.resolved_path);
        activate_concrete_descendants(index);
        if (target.status == OverrideItemTarget::Status::Existing) {
            if (!additions_.at(index).has_children)
                throw dip::SyntaxException(
                    "Empty override prefix", "The existing item `" + requested + "` has no override entries.",
                    "Add a value modification or a nested item.", __FILE__, __LINE__, line
                );
            return nullptr;
        }
        Line synthetic = line;
        synthetic.code = requested;
        Parser parser(synthetic);
        parser.part_path();
        auto group = GroupNode::is_node(parser);
        group->line = line;
        return group;
    }

} // namespace snt::dip
