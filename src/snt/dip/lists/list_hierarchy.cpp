#include <iostream>
#include <snt/dip/lists/list_hierarchy.h>
#include <sstream>

namespace snt::dip {

    void HierarchyList::record(const BaseNode::PointerType& node, const std::vector<NodeDtype>& excluded) {
        if (node->path.name == "")
            return;
        for (auto dtype : excluded)
            if (node->dtype == dtype)
                return;

        // closed children nodes and register new parent
        while (parents.size() > 0 && node->indent <= parents.back().indent)
            parents.pop_back();
        parents.push_back({node->indent, node->path.name, node->path.collections});

        // aggregate all path collections
        std::vector<Path::CollectionAccess> collections_full;
        collections_full.reserve(parents.size() * 2);
        for (const auto& parent : parents)
            collections_full.insert(collections_full.end(), parent.collections.begin(), parent.collections.end());
        if (collections_full.empty())
            throw std::runtime_error("Node does not belong to any collection: " + node->path.name);

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
            auto itc = collections.find(name_full);
            if (cnode.kind == Path::Kind::MAP) {
                if (itc == collections.end())
                    throw std::runtime_error("Collection does not exist: " + name_full);
                else if (std::find(itc->second.items.begin(), itc->second.items.end(), cnode.item) ==
                         itc->second.items.end())
                    throw std::runtime_error("Collection item does not exist: " + cnode.item);
                name_full += "[" + cnode.item + "]"; // use the key
            } else if (cnode.kind == Path::Kind::LIST) {
                if (itc == collections.end())
                    throw std::runtime_error("Collection does not exist: " + name_full);
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
            auto it = collections.find(name_full);
            if (cnode.kind == Path::Kind::MAP) {
                if (it == collections.end()) { // create new collection
                    collections[name_full] = Collection{name_full, {cnode.item}, Path::Kind::MAP};
                } else if (it->second.kind != Path::Kind::MAP) {
                    throw std::runtime_error("Collection cannot append keyed items: " + name_full);
                } else if (std::find(it->second.items.begin(), it->second.items.end(), cnode.item) ==
                           it->second.items.end()) { // append new item with a new key
                    it->second.items.push_back(cnode.item);
                } else {
                    throw std::runtime_error(
                        "Collection '" + name_full + "'already contains item with the key: " + cnode.item
                    );
                }
                name_full += "[" + cnode.item + "]";
            } else if (cnode.kind == Path::Kind::LIST) {
                std::string key;
                if (it == collections.end()) { // create new collection
                    key = "0";
                    collections[name_full] = Collection{name_full, {key}, Path::Kind::LIST};
                } else if (it->second.kind != Path::Kind::LIST) {
                    throw std::runtime_error("Collection cannot append indexed items: " + name_full);
                } else { // append new item with an increased index
                    key = std::to_string(it->second.items.size());
                    it->second.items.push_back(key);
                }
                name_full += "[" + key + "]";
            }
        }

        // register FQ name and collection set with the node
        // std::cout << '\n';
        node->path.name = name_full;
        // std::cout << "name  " << node->name << '\n';
        node->path.collections = std::move(collections_full);
    }

    const std::unordered_map<std::string, Collection>& HierarchyList::get_collections() const {
        return collections;
    }

    const Collection& HierarchyList::get_collection(const std::string& path) const {
        auto it = collections.find(path);
        if (it == collections.end()) {
            throw std::runtime_error("Collection does not exist: " + path);
        }
        return (it->second);
    }

} // namespace snt::dip
