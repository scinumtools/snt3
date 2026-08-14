#ifndef DIP_HIERARCHY_LIST_H
#define DIP_HIERARCHY_LIST_H

#include <snt/dip/nodes/node_base.h>
#include <unordered_map>

namespace snt::dip {

    /**
     * Holds information about a paraent node in the hierarchy
     */
    struct Parent {
        size_t indent;                                   ///< Indent of a parent node
        std::string name;                                ///< Fully qualified node path
        std::vector<Path::CollectionAccess> collections; ///< List of collections in the node path
    };

    /**
     * Information about a collection, keyed or indexed
     */
    struct Collection {
        std::string path;               ///< Fully qualified path of a collection
        std::vector<std::string> items; ///< List of collection items
        Path::Kind kind;                ///< Collection path kind
    };

    /**
     *  The main class that manages node hierarchy
     */
    class HierarchyList {
      private:
        std::vector<Parent> parents;                             ///< List of all parents of a current node hierarchy
        std::unordered_map<std::string, Collection> collections; ///< Map of all registered node collections

      public:
        /**
         * Register node into the node hierarchy
         *
         * @param node Node that should be registed
         * @param excluded List of node types that will be ignored in a node hierarchy
         */
        void record(const BaseNode::PointerType& node, const std::vector<NodeDtype>& excluded);

        /**
         * Get path of the current hierarchy level
         *
         * If indent and path is not provided, method returns the most recent path composed from
         * parents in the stack. When provided, the method returns path relative to the given
         * indent. E.g. if current path is 'foo.bar.baz' and group 'bar' has indent 2, than for
         * input indent 2 and path `crackle`, the following path will be constructed: `foo.crackle`.
         *
         * @param indent Indent of a new node
         * @param path Path name of a new node
         * @return Path of the current hierarchy level
         */
        const Path get_current_path(size_t indent = 0, const std::string& path = "") const;

        /**
         * Gets a reference on a ll collections
         *
         * @return Map of all collections
         */
        const std::unordered_map<std::string, Collection>& get_collections() const;

        /**
         * Get reference to a specific collection
         *
         * @param path Collection fully qualified path
         * @return Referenc on a collection
         */
        const Collection& get_collection(const std::string& path) const;

        /**
         * Set a new collection
         *
         * @param path Collection fully qualified path
         */
        void set_collection(const std::string& path, Path::Kind kind);

        /**
         * Get number of collections
         *
         * @return Number of collections
         */
        const size_t num_collections() const;
    };

} // namespace snt::dip

#endif // DIP_HIERARCHY_LIST_H
