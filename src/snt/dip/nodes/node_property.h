#ifndef DIP_NODE_PROPERTY_H
#define DIP_NODE_PROPERTY_H

#include <snt/dip/nodes/node_base.h>

namespace snt::dip {

    class PropertyNode : public virtual BaseNode {
      public:
        using PointerType = std::shared_ptr<PropertyNode>;
        PropertyType ptype;
        static BaseNode::PointerType is_node(Parser& parser);
        PropertyNode(Parser& parser, PropertyType pt) : BaseNode(parser, NodeDtype::Property), ptype(pt) {};
    };

} // namespace snt::dip

#endif // DIP_NODE_PROPERTY_H
