#ifndef DIP_LIST_SOURCE_H
#define DIP_LIST_SOURCE_H

#include <filesystem>
#include <map>
#include <snt/dip/lists/list_node.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/dip/settings.h>
#include <string>

namespace snt::dip {

    class SourceList; // EnvSource needs a forward declaration

    /** Named DIPL source text together with its parsed nodes. */
    struct EnvSource {
        std::string name;          // source key
        std::string path;          // source path
        std::string code;          // source code
        Source parent;             // parent source
        NodeList<ValueNode> nodes; // parsed nodes
                                   // std::shared_ptr<SourceList> sources;
    };

    /** Collection of named DIPL sources. */
    class SourceList {
      private:
        std::map<std::string, EnvSource> sources;

      public:
        SourceList();
        /** Append an item to the collection.
         * @param name Name used to identify the item.
         * @param path Path identifying the requested node or source.
         * @param code Source code associated with the source entry.
         * @param parent Parent source metadata used to preserve include provenance.
         */
        void append(
            const std::string& name, const std::filesystem::path& path, const std::string& code, const Source& parent
        );
        void append(const std::string& name, const EnvSource& src);
        EnvSource& at(const std::string& name);
        const EnvSource& at(const std::string& name) const;
    };

} // namespace snt::dip

#endif // DIP_LIST_SOURCE_H
