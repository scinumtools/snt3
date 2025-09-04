#ifndef DIP_NODE_CASE_H
#define DIP_NODE_CASE_H

#include "node_base.h"

namespace snt::dip {

  class Environment;

  class CaseNode : public BaseNode {
  public:
    bool value;         // case value
    size_t case_id;     // ID of a case
    CaseType case_type; // type of a case
    static BaseNode::PointerType is_node(Parser& parser);
    CaseNode(Parser& parser) : BaseNode(parser, NodeDtype::Case), case_id(0), value(false) {};
    BaseNode::NodeListType parse(Environment& env) override;
  };

} // namespace snt::dip

#endif // DIP_NODE_CASE_H
