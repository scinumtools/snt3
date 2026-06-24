#ifndef DIP_HIERARCHY_LIST_H
#define DIP_HIERARCHY_LIST_H

#include <snt/dip/nodes/node_base.h>
#include <unordered_map>

namespace snt::dip {

    /**
     * @struct Parent
     * @brief Holds information about a paraent node in the hierarchy
     */
    struct Parent {
        size_t indent;                                   ///< Indent of a parent node
        std::string name;                                ///< Fully qualified node path
        std::vector<Path::CollectionAccess> collections; ///< List of collections in the node path
    };

    /**
     * @struct Collection
     * @brief Information about a collection, keyed or indexed
     */
    struct Collection {
        std::string path;               ///< Fully qualified path of a collection
        std::vector<std::string> items; ///< List of collection items
        Path::CollectionType type;      ///< Collection type
    };

    /**
     * @class HierarchyList
     * @brief The main class that manages node hierarchy
     */
    class HierarchyList {
      private:
        std::vector<Parent> parents;                             ///< List of all parents of a current node hierarchy
        std::unordered_map<std::string, Collection> collections; ///< Map of all registered node collections

      public:
        /**
         * @brief Register node into the node hierarchy
         *
         * @param node Node that should be registed
         * @param excluded List of node types that will be ignored in a node hierarchy
         */
        void record(const BaseNode::PointerType& node, const std::vector<NodeDtype>& excluded);

        /**
         * @brief Gets a reference on a ll collections
         *
         * @return Map of all collections
         */
        const std::unordered_map<std::string, Collection>& get_collections() const;

        /**
         * @brief Get reference to a specific collection
         *
         * @param path Collection fully qualified path
         * @return Referenc on a collection
         */
        const Collection& get_collection(const std::string& path) const;
    };

} // namespace snt::dip

#endif // DIP_HIERARCHY_LIST_H
