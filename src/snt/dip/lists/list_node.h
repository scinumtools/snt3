#ifndef DIP_LIST_NODE_H
#define DIP_LIST_NODE_H

#include "../../val.h"
#include "../environment.h"
#include "../nodes/node_base.h"
#include "../nodes/parser.h"

namespace snt::dip {

  // Node List

  class NodeList {
  private:
    BaseNode::NodeListType nodes;

  public:
    NodeList() {};
    NodeList(const BaseNode::NodeListType& nl) : nodes(nl) {};
    size_t size() const;
    void push_front(const BaseNode::PointerType& node);
    void push_back(const BaseNode::PointerType& node);
    BaseNode::PointerType pop_front();
    BaseNode::PointerType pop_back();
    BaseNode::PointerType at(const size_t index);
    BaseNode::PointerType at(const size_t index) const;
  };

} // namespace snt::dip

#endif // DIP_LIST_NODE_H
