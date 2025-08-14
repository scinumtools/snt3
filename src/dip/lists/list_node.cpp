#include "lists.h"

namespace dip {

  size_t NodeList::size() const {
    return nodes.size();
  }

  void NodeList::push_front(BaseNode::PointerType node) {
    nodes.push_front(node);
  }

  void NodeList::push_back(BaseNode::PointerType node) {
    nodes.push_back(node);
  }

  BaseNode::PointerType NodeList::pop_front() {
    BaseNode::PointerType node = nodes.front();
    nodes.pop_front();
    return node;
  }

  BaseNode::PointerType NodeList::pop_back() {
    BaseNode::PointerType node = nodes.back();
    nodes.pop_back();
    return node;
  }

  BaseNode::PointerType NodeList::at(const size_t index) {
    return nodes.at(index);
  }

  BaseNode::PointerType NodeList::at(const size_t index) const {
    return nodes.at(index);
  }

} // namespace dip
