#ifndef DIP_LIST_SCHEMA_H
#define DIP_LIST_SCHEMA_H

#include <map>
#include <snt/dip/lists/list_node.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/dip/settings.h>
#include <string>

namespace snt::dip {

    struct EnvSchema {
        std::string name;          // schema key
        ValueNode::ListType nodes; // aggregated nodes
    };

    class SchemaList {
      private:
        std::map<std::string, EnvSchema> schemas;

      public:
        SchemaList();
        void append(const std::string& name, ValueNode::ListType& nodes);
        void append(const std::string& name, const EnvSchema& src);
        EnvSchema& at(const std::string& name);
        const EnvSchema& at(const std::string& name) const;
    };

} // namespace snt::dip

#endif // DIP_LIST_SCHEMA_H
