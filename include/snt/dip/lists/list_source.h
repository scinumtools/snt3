#ifndef DIP_LIST_SOURCE_H
#define DIP_LIST_SOURCE_H

#include <filesystem>
#include <map>
#include <snt/dip/inspect/declarations.h>
#include <snt/dip/lists/list_hierarchy.h>
#include <snt/dip/lists/list_node.h>
#include <snt/dip/lists/list_schema.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/dip/settings.h>
#include <string>
#include <vector>

namespace snt::dip {

    class SourceList; // EnvSource needs a forward declaration

    /**
     * Named DIPL source text together with its parsed nodes.
     */
    struct EnvSource {
        std::string name;                  ///< Source lookup key.
        std::string path;                  ///< Original source path.
        std::string code;                  ///< Exact source text.
        Source parent;                     ///< Source that introduced this entry.
        NodeList<ValueNode> nodes;         ///< Parsed source nodes.
        bool named_source = false;         ///< Whether source?path references can use this entry.
        bool raw_text = false;             ///< Whether content is kept verbatim instead of parsed as DIPL.
        bool table_text = false;           ///< Whether raw content was parsed as a table.
        std::string table_path;            ///< Resolved table populated by this raw source, if any.
        HierarchyList hierarchy;           ///< Hierarchy local to this source.
        SchemaList schemas;                ///< Schemas available to imported collections.
        ExplicitDeclarations declarations; ///< Explicit source-local paths.
        bool embedded = false;             ///< Whether code came from memory rather than path.
        ValueMetadata metadata;            ///< Metadata declared at the source root.
                                           // std::shared_ptr<SourceList> sources;
    };

    /**
     * Collection of named DIPL sources.
     */
    class SourceList {
      private:
        std::map<std::string, EnvSource> sources; ///< Named entries keyed for lookup.
        // The map supports named lookup; v3 archives also need parse-time order.
        std::vector<std::string> registration_order_; ///< Source names in parse order.

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

        /**
         * Return all registered sources keyed by their DIPL source name.
         */
        const std::map<std::string, EnvSource>& entries() const;
        /**
         * Source names in the order they were registered during parsing.
         */
        const std::vector<std::string>& registration_order() const { return registration_order_; }
    };

} // namespace snt::dip

#endif // DIP_LIST_SOURCE_H
