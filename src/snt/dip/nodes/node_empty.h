#ifndef DIP_NODE_EMPTY_H
#define DIP_NODE_EMPTY_H

#include <snt/dip/nodes/node_base.h>

namespace snt::dip {

  class EmptyNode : public BaseNode {
  public:
    static BaseNode::PointerType is_node(Parser& parser);
    EmptyNode(Parser& parser) : BaseNode(parser, NodeDtype::Empty) {};
  };

} // namespace snt::dip

#endif // DIP_NODE_EMPTY_H
