#ifndef DIP_NODE_SCHEMA_H
#define DIP_NODE_SCHEMA_H

#include <snt/dip/nodes/node_value.h>

namespace snt::dip {

    class SchemaNode : public BaseNode {
      public:
        ValueMetadata metadata; ///< Metadata describing the schema definition.
        static BaseNode::PointerType is_node(Parser& parser);
        SchemaNode(Parser& parser) : BaseNode(parser, NodeDtype::Schema) {};
        BaseNode::ListType parse(Environment& env) override;
        bool set_property(PropertyType property, val::Array::StringType& values, std::string& units) override;
    };

} // namespace snt::dip

#endif // DIP_NODE_SCHEMA_H
