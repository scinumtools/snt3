#ifndef DIP_PATH_H
#define DIP_PATH_H

#include <map>
#include <string>
#include <vector>

namespace snt::dip {

    /**
     * @class Path
     * @brief This class manages fully qualified node paths
     */
    class Path {
      public:
        /**
         * @brief List of collection types
         */
        enum class CollectionType {
            NONE,  ///< Node is not a collection
            GROUP, ///< Simple node or group
            MAP,   ///< Map items are associated with a key
            LIST,  ///< List items are ordered by indexes
        };

        /**
         * @struct CollectionAccess
         * @brief Collection node information
         */
        struct CollectionAccess {
            std::string path;    ///< Paths leading to a collection
            std::string item;    ///< Item value
            CollectionType type; ////< Collection type
        };

        std::string name;                          ///< fully qualified node path
        std::vector<CollectionAccess> collections; ///< list of collection access pairs derived from a node name

        /**
         * @brief Class constructor
         */
        Path(const std::string& path);
    };

} // namespace snt::dip

#endif // DIP_PATH_H
