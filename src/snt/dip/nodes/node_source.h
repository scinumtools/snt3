#ifndef DIP_NODE_SOURCE_H
#define DIP_NODE_SOURCE_H

#include <snt/dip/nodes/node_base.h>

namespace snt::dip {

  class SourceNode : public BaseNode {
  public:
    static BaseNode::PointerType is_node(Parser& parser);
    SourceNode(Parser& parser) : BaseNode(parser, NodeDtype::Source) {};
    BaseNode::ListType parse(Environment& env) override;
  };

} // namespace snt::dip

#endif // DIP_NODE_SOURCE_H
