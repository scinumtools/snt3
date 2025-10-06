#ifndef DIP_LIST_NODE_H
#define DIP_LIST_NODE_H

#include "../../val.h"
#include "../nodes/node_base.h"
#include "../nodes/parser.h"

namespace snt::dip {

  // Node List
  template <typename T>
  class NodeList {
  private:
    T::ListType nodes;

  public:
    NodeList() {};
    NodeList(const T::ListType& nl) : nodes(nl) {};
    size_t size() const {
      return nodes.size();
    };
    void push_front(const T::PointerType& node) {
      nodes.push_front(node);
    };
    void push_back(const T::PointerType& node) {
      nodes.push_back(node);
    };
    T::PointerType pop_front() {
      BaseNode::PointerType node = nodes.front();
      nodes.pop_front();
      return node;
    };
    T::PointerType pop_back() {
      BaseNode::PointerType node = nodes.back();
      nodes.pop_back();
      return node;
    };
    T::PointerType at(const size_t index) {
      return nodes.at(index);
    };
    T::PointerType at(const size_t index) const {
      return nodes.at(index);
    };
  };

} // namespace snt::dip

#endif // DIP_LIST_NODE_H
