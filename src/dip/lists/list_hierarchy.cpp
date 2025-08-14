#include <iostream>
#include <sstream>

#include "lists.h"

namespace dip {

  void HierarchyList::record(BaseNode::PointerType node, const std::vector<NodeDtype>& excluded) {
    if (node->name == "")
      return;
    for (auto dtype : excluded)
      if (node->dtype == dtype)
        return;

    // closed children nodes and register new parent
    while (parents.size() > 0 and node->indent <= parents.back().indent) {
      parents.pop_back();
    }
    parents.push_back({node->indent, node->name});

    // set node name according to the hierarchy
    if (parents.size() > 1) {
      std::stringstream ss;
      for (auto parent : parents)
        ss << parent.name << SIGN_SEPARATOR;
      node->name = ss.str();
      node->name = node->name.substr(0, node->name.size() - 1);
    }
  }

} // namespace dip
