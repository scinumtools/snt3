#ifndef DIP_HIERARCHY_LIST_H
#define DIP_HIERARCHY_LIST_H

#include "../nodes/node_base.h"

namespace snt::dip {

  struct Parent {
    size_t indent;
    std::string name;
  };

  class HierarchyList {
  private:
    std::vector<Parent> parents;

  public:
    void record(const BaseNode::PointerType& node, const std::vector<NodeDtype>& excluded);
  };

} // namespace snt::dip

#endif // DIP_HIERARCHY_LIST_H
