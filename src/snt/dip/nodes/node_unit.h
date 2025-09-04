#ifndef DIP_NODE_UNIT_H
#define DIP_NODE_UNIT_H

namespace snt::dip {

  class UnitNode : public BaseNode {
  public:
    static BaseNode::PointerType is_node(Parser& parser);
    UnitNode(Parser& parser) : BaseNode(parser, NodeDtype::Unit) {};
    BaseNode::NodeListType parse(Environment& env) override;
  };

} // namespace snt::dip

#endif // DIP_NODE_UNIT_H
