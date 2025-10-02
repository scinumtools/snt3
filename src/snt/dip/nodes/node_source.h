#ifndef DIP_NODE_SOURCE_H
#define DIP_NODE_SOURCE_H

#include "node_base.h"

namespace snt::dip {

  class SourceNode : public BaseNode {
  public:
    static BaseNode::PointerType is_node(Parser& parser);
    SourceNode(Parser& parser) : BaseNode(parser, NodeDtype::Source) {};
    BaseNode::NodeListType parse(Environment& env) override;
  };

} // namespace snt::dip

#endif // DIP_NODE_SOURCE_H
