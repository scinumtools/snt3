#ifndef DIP_PATH_H
#define DIP_PATH_H

#include <map>
#include <string>
#include <vector>

namespace snt::dip {

    /**
     * This class manages fully qualified node paths
     */
    class Path {
      public:
        /**
         * List of collection types
         */
        enum class Kind {
            None,  ///< Node is not a collection
            Empty, ///< Path is empty, or root
            Group, ///< Simple node or group
            Map,   ///< Map items are associated with a key
            List,  ///< List items are ordered by indexes
            Item,  ///< Item in a map or list collection
        };

        /**
         * Collection node information
         */
        struct CollectionAccess {
            std::string path; ///< Paths leading to a collection
            std::string item; ///< Item value
            Kind kind;        ///< Kind of a collection path
        };

        std::string name;                                ///< fully qualified node path
        Kind kind;                                       ///< kind of a path
        std::vector<Path::CollectionAccess> collections; ///< list of collection access pairs derived from a node name

        /**
         * Class constructor
         */
        Path() = default;

        /**
         * Class constructor
         *
         * @param path Path that will be parsed
         */
        Path(const std::string& path);
    };

} // namespace snt::dip

#endif // DIP_PATH_H
