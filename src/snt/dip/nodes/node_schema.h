#ifndef DIP_NODE_SCHEMA_H
#define DIP_NODE_SCHEMA_H

#include <snt/dip/nodes/node_base.h>

namespace snt::dip {

    class SchemaNode : public BaseNode {
      public:
        static BaseNode::PointerType is_node(Parser& parser);
        SchemaNode(Parser& parser) : BaseNode(parser, NodeDtype::Schema) {};
        BaseNode::ListType parse(Environment& env) override;
    };

} // namespace snt::dip

#endif // DIP_NODE_SCHEMA_H
