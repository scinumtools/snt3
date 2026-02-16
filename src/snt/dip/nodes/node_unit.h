#ifndef DIP_NODE_UNIT_H
#define DIP_NODE_UNIT_H

#include <snt/dip/nodes/node_base.h>

namespace snt::dip {

  class UnitNode : public BaseNode {
  public:
    static BaseNode::PointerType is_node(Parser& parser);
    UnitNode(Parser& parser) : BaseNode(parser, NodeDtype::Unit) {};
    BaseNode::ListType parse(Environment& env) override;
  };

} // namespace snt::dip

#endif // DIP_NODE_UNIT_H
