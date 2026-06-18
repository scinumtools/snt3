#ifndef DIP_HIERARCHY_LIST_H
#define DIP_HIERARCHY_LIST_H

#include <snt/dip/nodes/node_base.h>
#include <unordered_map>

namespace snt::dip {

    struct Parent {
        size_t indent;
        std::string name;
        std::vector<Path::CollectionAccess> collections;
    };

    struct Collection {
        std::string path;
        std::vector<std::string> items;
        Path::CollectionType type;
    };

    class HierarchyList {
      private:
        std::vector<Parent> parents;
        std::unordered_map<std::string, Collection> collections;

      public:
        void record(const BaseNode::PointerType& node, const std::vector<NodeDtype>& excluded);
        const std::unordered_map<std::string, Collection>& get_collections() const { return collections; };
    };

} // namespace snt::dip

#endif // DIP_HIERARCHY_LIST_H
