#include <algorithm>
#include <cctype>
#include <iostream>
#include <snt/dip/exceptions.h>
#include <snt/dip/lists/list_hierarchy.h>
#include <utility>

namespace snt::dip {
    namespace {
        bool in_subtree(const std::string& path, const std::string& root) {
            return root.empty() || path == root ||
                   (path.size() > root.size() && path.compare(0, root.size(), root) == 0 &&
                    (path[root.size()] == SIGN_SEPARATOR || path[root.size()] == SIGN_ARRAY_OPEN));
        }

        bool is_index_selector(const std::string& item) {
            return !item.empty() && std::all_of(item.begin(), item.end(), [](unsigned char character) {
                return std::isdigit(character);
            });
        }

        void validate_list_index(
            const Collection& collection, const std::string& path, const std::string& item, const Line& line
        ) {
            const auto index = std::stoull(item);
            if (index >= collection.items.size())
                throw dip::EnvironmentException(
                    "Unknown collection item",
                    "The list index `" + item + "` is outside collection `" + path + "`.",
                    "Use an existing list index or append a new item with `[]`.",
                    __FILE__,
                    __LINE__,
                    line
                );
        }
    } // namespace

    void HierarchyList::record_parent(const BaseNode::PointerType& node) {
        while (!parents.empty() && node->indent <= parents.back().indent)
            parents.pop_back();
        parents.push_back({node->indent, node->path.name, node->path.collections});
    }

    void HierarchyList::record(
        const BaseNode::PointerType& node,
        const std::vector<NodeDtype>& excluded,
        const std::function<std::string(const std::string&)>& collection_path
    ) {
        if (node->path.name == "")
            return;
        for (auto dtype : excluded)
            if (node->dtype == dtype)
                return;
        const auto key_for = [&](const std::string& path) {
            return collection_path ? collection_path(path) : path;
        };
        const auto contains_item = [&](const Collection& collection, const std::string& path, const std::string& item) {
            if (collection.kind == Path::Kind::Map) {
                const auto lookup = map_item_lookup.find(path);
                return lookup != map_item_lookup.end() && lookup->second.find(item) != lookup->second.end();
            }
            return std::find(collection.items.begin(), collection.items.end(), item) != collection.items.end();
        };

        // closed children nodes and register new parent
        record_parent(node);

        // aggregate all path collections
        std::vector<Path::CollectionAccess> collections_full;
        collections_full.reserve(parents.size() * 2);
        for (const auto& parent : parents)
            collections_full.insert(collections_full.end(), parent.collections.begin(), parent.collections.end());
        if (collections_full.empty())
            throw dip::EnvironmentException(
                "Missing path collection",
                "The node path `" + node->path.name + "` does not belong to any collection.",
                "Check the node path and make sure it is associated with a valid collection.",
                __FILE__,
                __LINE__,
                node->line
            );

        //  resolve fully qualified (FQ) names of all but last collection
        std::string name_full;
        // std::cout << "colls ";
        auto it = collections_full.begin();
        for (; it != std::prev(collections_full.end()); ++it) {
            const auto& cnode = *it;
            // std::cout << cnode.path << " " << cnode.item << " | ";
            //  append FQ path
            if (!name_full.empty())
                name_full += SIGN_SEPARATOR;
            name_full += cnode.path;
            // append FQ item selector and test if parent collections and item key exist
            const std::string key = key_for(name_full);
            auto itc = collections.find(key);
            if (cnode.kind == Path::Kind::Map) {
                if (itc == collections.end())
                    throw dip::EnvironmentException(
                        "Unknown collection",
                        "The collection `" + name_full + "` was not found.",
                        "Check whether the collection path is correct.",
                        __FILE__,
                        __LINE__,
                        node->line
                    );
                else if (itc->second.kind == Path::Kind::List && is_index_selector(cnode.item)) {
                    validate_list_index(itc->second, key, cnode.item, node->line);
                } else if (!contains_item(itc->second, key, cnode.item))
                    throw dip::EnvironmentException(
                        "Unknown collection item",
                        "The item `" + cnode.item + "` was not found in the collection `" + name_full + "`.",
                        "Check whether the collection item name is correct.",
                        __FILE__,
                        __LINE__,
                        node->line
                    );
                name_full += "[" + cnode.item + "]"; // use the key
            } else if (cnode.kind == Path::Kind::List) {
                if (itc == collections.end())
                    throw dip::EnvironmentException(
                        "Unknown collection",
                        "The collection `" + name_full + "` was not found.",
                        "Check whether the collection path is correct.",
                        __FILE__,
                        __LINE__,
                        node->line
                    );
                name_full += "[" + itc->second.items.back() + "]"; // use index of the most recent item
            }
        }

        // resolve FQ names of the last collection
        it = std::prev(collections_full.end());
        {
            const auto& cnode = *it;
            // std::cout << cnode.path << " " << cnode.item << " | ";
            //  append FQ path
            if (!name_full.empty())
                name_full += SIGN_SEPARATOR;
            name_full += cnode.path;
            // append FQ item selector and register new collections
            const std::string key = key_for(name_full);
            auto it = collections.find(key);
            if (cnode.kind == Path::Kind::Map) {
                if (it != collections.end() && it->second.kind == Path::Kind::List && is_index_selector(cnode.item)) {
                    validate_list_index(it->second, key, cnode.item, node->line);
                    name_full += "[" + cnode.item + "]";
                } else if (it == collections.end()) { // create new collection
                    collections[key] = Collection{key, {cnode.item}, Path::Kind::Map, {}};
                    map_item_lookup[key].insert(cnode.item);
                } else if (it->second.kind != Path::Kind::Map) {
                    throw dip::EnvironmentException(
                        "Invalid collection type",
                        "The collection `" + name_full + "` is not a map.",
                        "Use a map collection when appending keyed items.",
                        __FILE__,
                        __LINE__,
                        node->line
                    );
                } else if (!contains_item(it->second, key, cnode.item)) {
                    // append new item with a new key
                    it->second.items.push_back(cnode.item);
                    map_item_lookup.at(key).insert(cnode.item);
                } else {
                    throw dip::EnvironmentException(
                        "Duplicate collection item",
                        "The collection `" + name_full + "` already contains the key `" + cnode.item + "`.",
                        "Choose a different key for the collection item.",
                        __FILE__,
                        __LINE__,
                        node->line
                    );
                }
                if (name_full.back() != ']') {
                    name_full += "[" + cnode.item + "]";
                    const std::string item_key = key_for(name_full);
                    collections[item_key] = Collection{item_key, {}, Path::Kind::Item, {}};
                }
            } else if (cnode.kind == Path::Kind::List) {
                std::string item_index;
                if (it == collections.end()) { // create new collection
                    item_index = "0";
                    collections[key] = Collection{key, {item_index}, Path::Kind::List, {}};
                } else if (it->second.kind != Path::Kind::List) {
                    throw dip::EnvironmentException(
                        "Invalid collection type",
                        "The collection `" + name_full + "` is not a list.",
                        "Use a list collection when appending indexed items.",
                        __FILE__,
                        __LINE__,
                        node->line
                    );
                } else { // append new item with an increased index
                    item_index = std::to_string(it->second.items.size());
                    it->second.items.push_back(item_index);
                }
                name_full += "[" + item_index + "]";
                const std::string item_key = key_for(name_full);
                collections[item_key] = Collection{item_key, {}, Path::Kind::Item, {}};
            } else if (cnode.kind == Path::Kind::Group) {
                auto col = collections.find(key);
                if (col == collections.end()) {
                    collections[key] = Collection{key, {}, Path::Kind::Group, {}};
                }
            }
        }

        // register FQ name and collection set with the node
        // std::cout << '\n';
        node->path.name = name_full;
        if (node->dtype == NodeDtype::Group && !node->metadata_constraints.empty()) {
            const auto collection = collections.find(key_for(name_full));
            if (collection != collections.end())
                collection->second.metadata_constraints.insert(
                    collection->second.metadata_constraints.end(),
                    node->metadata_constraints.begin(),
                    node->metadata_constraints.end()
                );
        }
        if (collection_path)
            record_ordered_path(collection_path(name_full));
        // std::cout << "name  " << node->name << '\n';
        node->path.collections = std::move(collections_full);
    }

    const Path HierarchyList::get_current_path(size_t indent, const std::string& path, bool show_item) const {
        std::string new_path;
        new_path.reserve(path.size() + parents.size() * 16);
        for (size_t parent = 0; parent < parents.size(); parent++) {
            if (parents[parent].indent >= indent)
                continue;
            if (parent > 0)
                new_path += SIGN_SEPARATOR;
            new_path += parents[parent].name;
        }
        std::string clean_path = path;
        if (!show_item && !clean_path.empty() && clean_path.back() == ']') {
            const auto pos = clean_path.rfind('[');
            if (pos != std::string::npos)
                clean_path.erase(pos);
        }
        if (!clean_path.empty()) {
            if (!new_path.empty())
                new_path += SIGN_SEPARATOR;
            new_path += clean_path;
        }
        return Path(new_path);
    }

    std::string HierarchyList::resolve_list_selectors(std::string path) const {
        size_t position = 0;
        while ((position = path.find("[]", position)) != std::string::npos) {
            const std::string collection_path = path.substr(0, position);
            const auto collection = collections.find(collection_path);
            if (collection == collections.end() || collection->second.items.empty())
                break;
            const std::string& index = collection->second.items.back();
            path.replace(position, 2, "[" + index + "]");
            position += index.size() + 2;
        }
        return path;
    }

    const std::unordered_map<std::string, Collection>& HierarchyList::get_collections() const {
        return collections;
    }

    const Collection& HierarchyList::get_collection(const std::string& path) const {
        auto it = collections.find(path);
        if (it == collections.end()) {
            throw dip::EnvironmentException(
                "Unknown collection",
                "The collection `" + path + "` was not found in the environment hierarchy.",
                "Check whether the collection path is correct.",
                __FILE__,
                __LINE__
            );
        }
        return (it->second);
    }

    void HierarchyList::set_collection(const std::string& path, Path::Kind kind, std::vector<std::string> schemas) {
        if (collections.find(path) != collections.end())
            throw dip::EnvironmentException(
                "Duplicate collection",
                "A collection with the path `" + path + "` already exists in the environment hierarchy.",
                "Choose a different collection path.",
                __FILE__,
                __LINE__
            );
        collections[path] = Collection{path, {}, kind, std::move(schemas)};
        if (kind == Path::Kind::Map)
            map_item_lookup[path];
    }

    void HierarchyList::set_collection(
        const std::string& path, Path::Kind kind, std::vector<std::string> schemas, std::vector<std::string> items
    ) {
        if (collections.find(path) != collections.end())
            throw dip::EnvironmentException(
                "Duplicate collection",
                "A collection with the path `" + path + "` already exists in the environment hierarchy.",
                "Choose a different collection path.",
                __FILE__,
                __LINE__
            );
        collections[path] = Collection{path, std::move(items), kind, std::move(schemas)};
        if (kind == Path::Kind::Map) {
            auto& lookup = map_item_lookup[path];
            const auto& restored_items = collections.at(path).items;
            lookup.insert(restored_items.begin(), restored_items.end());
        }
    }

    void HierarchyList::import_subtree(
        const HierarchyList& source, const std::string& source_root, const std::string& target_root
    ) {
        const std::string destination = target_root.empty() ? source_root : target_root;
        std::vector<Collection> selected;
        for (const auto& [path, collection] : source.collections) {
            if (!in_subtree(path, source_root))
                continue;
            std::string mapped;
            if (source_root.empty()) {
                mapped = destination.empty() ? path : destination + std::string(1, SIGN_SEPARATOR) + path;
            } else if (path == source_root) {
                mapped = destination;
            } else {
                mapped = destination + path.substr(source_root.size());
            }
            if (mapped.empty())
                continue;
            Collection copy = collection;
            copy.path = mapped;
            selected.push_back(std::move(copy));
        }

        // A whole-source import introduces its destination group even when the
        // source's first evaluated value is inside a keyed collection.
        if (source_root.empty() && !destination.empty() && !has_collection(destination))
            collections[destination] = Collection{destination, {}, Path::Kind::Group, {}};
        if (!destination.empty())
            record_ordered_path(destination);

        for (auto& copy : selected) {
            const auto existing = collections.find(copy.path);
            if (existing == collections.end()) {
                if (copy.kind == Path::Kind::Map) {
                    auto& lookup = map_item_lookup[copy.path];
                    lookup.insert(copy.items.begin(), copy.items.end());
                }
                collections.emplace(copy.path, std::move(copy));
                continue;
            }
            if (existing->second.kind != copy.kind)
                throw dip::EnvironmentException(
                    "Invalid imported collection type",
                    "The imported collection `" + copy.path + "` has a different type from the existing collection.",
                    "Import it under a different path or use collections of the same type.",
                    __FILE__, __LINE__
                );
            const auto append_unique = [](auto& target, const auto& values) {
                for (const auto& value : values)
                    if (std::find(target.begin(), target.end(), value) == target.end())
                        target.push_back(value);
            };
            append_unique(existing->second.items, copy.items);
            if (existing->second.kind == Path::Kind::Map) {
                auto& lookup = map_item_lookup[copy.path];
                lookup.insert(existing->second.items.begin(), existing->second.items.end());
            }
            append_unique(existing->second.children, copy.children);
            append_unique(existing->second.schemas, copy.schemas);
        }
    }

    std::vector<std::string> HierarchyList::subtree_schemas(const std::string& source_root) const {
        std::vector<std::string> result;
        for (const auto& [path, collection] : collections) {
            if (!in_subtree(path, source_root))
                continue;
            for (const auto& schema : collection.schemas)
                if (std::find(result.begin(), result.end(), schema) == result.end())
                    result.push_back(schema);
        }
        std::sort(result.begin(), result.end());
        return result;
    }

    void HierarchyList::set_schemas(const std::string& path, std::vector<std::string> schemas) {
        get_collection(path);
        collections.at(path).schemas = std::move(schemas);
    }

    void HierarchyList::record_ordered_path(const std::string& path) {
        if (path.empty())
            return;
        const auto append_unique = [](std::vector<std::string>& values, const std::string& value) {
            if (std::find(values.begin(), values.end(), value) == values.end())
                values.push_back(value);
        };

        std::string current;
        size_t position = 0;
        while (position < path.size()) {
            if (path[position] == '.') {
                ++position;
                continue;
            }
            if (path[position] == '[') {
                const size_t end = path.find(']', position);
                if (end == std::string::npos)
                    return;
                current += path.substr(position, end - position + 1);
                position = end + 1;
                continue;
            }

            const size_t start = position;
            while (position < path.size() && path[position] != '.' && path[position] != '[')
                ++position;
            const std::string name = path.substr(start, position - start);
            if (name.empty())
                return;

            if (current.empty()) {
                append_unique(root_children, name);
                current = name;
                continue;
            }

            const auto parent = collections.find(current);
            if (parent != collections.end() &&
                (parent->second.kind == Path::Kind::Group || parent->second.kind == Path::Kind::Item))
                append_unique(parent->second.children, name);
            current += SIGN_SEPARATOR + name;
        }
    }

    void HierarchyList::merge_metadata(const std::string& path, const ValueMetadata& metadata) {
        if (!has_collection(path))
            collections[path] = Collection{path, {}, Path::Kind::Group, {}};
        collections.at(path).metadata.merge_explicit(metadata);
    }

    void HierarchyList::erase_collection(const std::string& path) {
        collections.erase(path);
        map_item_lookup.erase(path);
    }

    const bool HierarchyList::has_collection(const std::string& path) const {
        return collections.find(path) != collections.end();
    }

    const size_t HierarchyList::num_collections() const {
        return collections.size();
    }

} // namespace snt::dip
