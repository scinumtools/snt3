#ifndef DIP_NODE_OVERRIDE_H
#define DIP_NODE_OVERRIDE_H

#include <snt/dip/nodes/node_base.h>

namespace snt::dip {

    /** Marker for a region of value-only modifications collected before evaluation. */
    class OverrideNode : public BaseNode {
      public:
        static BaseNode::PointerType is_node(Parser& parser);
        OverrideNode(Parser& parser) : BaseNode(parser, NodeDtype::Override) {}
    };

} // namespace snt::dip

#endif // DIP_NODE_OVERRIDE_H
