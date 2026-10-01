#include <algorithm>
#include <cctype>
#include <iostream>
#include <snt/dip/exceptions.h>
#include <snt/dip/lists/list_hierarchy.h>
#include <sstream>
#include <utility>

namespace snt::dip {
    namespace {
        bool is_index_selector(const std::string& item) {
            return !item.empty() &&
                   std::all_of(item.begin(), item.end(), [](unsigned char character) { return std::isdigit(character); });
        }

        void validate_list_index(const Collection& collection, const std::string& path, const std::string& item, const Line& line) {
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
        const BaseNode::PointerType& node, const std::vector<NodeDtype>& excluded,
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
                } else if (
                    std::find(itc->second.items.begin(), itc->second.items.end(), cnode.item) == itc->second.items.end()
                )
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
                } else if (it->second.kind != Path::Kind::Map) {
                    throw dip::EnvironmentException(
                        "Invalid collection type",
                        "The collection `" + name_full + "` is not a map.",
                        "Use a map collection when appending keyed items.",
                        __FILE__,
                        __LINE__,
                        node->line
                    );
                } else if (
                    std::find(it->second.items.begin(), it->second.items.end(), cnode.item) == it->second.items.end()
                ) { // append new item with a new key
                    it->second.items.push_back(cnode.item);
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
        // std::cout << "name  " << node->name << '\n';
        node->path.collections = std::move(collections_full);
    }

    const Path HierarchyList::get_current_path(size_t indent, const std::string& path, bool show_item) const {
        std::stringstream new_path;
        for (size_t parent = 0; parent < parents.size(); parent++) {
            if (parents[parent].indent >= indent)
                continue;
            if (parent > 0)
                new_path << SIGN_SEPARATOR;
            new_path << parents[parent].name;
        }
        std::string clean_path = path;
        if (!show_item && !clean_path.empty() && clean_path.back() == ']') {
            const auto pos = clean_path.rfind('[');
            if (pos != std::string::npos)
                clean_path.erase(pos);
        }
        if (!clean_path.empty()) {
            if (!new_path.str().empty())
                new_path << SIGN_SEPARATOR;
            new_path << clean_path;
        }
        return Path(new_path.str());
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
    }

    void HierarchyList::set_schemas(const std::string& path, std::vector<std::string> schemas) {
        get_collection(path);
        collections.at(path).schemas = std::move(schemas);
    }

    void HierarchyList::erase_collection(const std::string& path) {
        collections.erase(path);
    }

    const bool HierarchyList::has_collection(const std::string& path) const {
        return collections.find(path) != collections.end();
    }

    const size_t HierarchyList::num_collections() const {
        return collections.size();
    }

} // namespace snt::dip
