#ifndef DIP_LIST_SCHEMA_H
#define DIP_LIST_SCHEMA_H

#include <map>
#include <snt/dip/lists/list_node.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/dip/settings.h>
#include <string>

namespace snt::dip {

    /** Reusable schema definition registered in a parsed DIPL environment. */
    struct EnvSchema {
        std::string name;         // schema key
        BaseNode::ListType nodes; // aggregated nodes
        std::string id;           // internal trace identifier, e.g. DIP0_SCHEMA0
        ValueMetadata metadata;   ///< Metadata on the definition, not inherited by instances.
        std::string source_name;  ///< Source containing the declaration or schema body.
        size_t source_line = 0;   ///< Declaration line, or first body line for host schemas.
    };

    class SchemaList {
      private:
        std::map<std::string, EnvSchema> schemas;
        std::map<std::string, size_t> id_counters;

      public:
        SchemaList();
        /** Append a schema definition to the registry.
         * @param name Name used to identify the item.
         * @param nodes Nodes defining the schema.
         * @param parent_id Internal identifier of the registering source.
         * @param metadata Metadata attached to the schema definition.
         */
        void append(
            const std::string& name, BaseNode::ListType& nodes, const std::string& parent_id = {},
            const ValueMetadata& metadata = {}, size_t source_line = 0
        );
        /** Append a schema record, assigning an identifier when it has none. */
        void append(const std::string& name, EnvSchema src, const std::string& parent_id = {});
        EnvSchema& at(const std::string& name);
        const EnvSchema& at(const std::string& name) const;

        /** Return all registered schema definitions and metadata keyed by name. */
        const std::map<std::string, EnvSchema>& entries() const;
    };

} // namespace snt::dip

#endif // DIP_LIST_SCHEMA_H
