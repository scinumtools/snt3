#ifndef DIP_ENVIRONMENT_H
#define DIP_ENVIRONMENT_H

#include "lists/list_source.h"
#include "lists/list_unit.h"
#include "lists/list_node.h"
#include "lists/list_hierarchy.h"
#include "lists/list_branching.h"
#include "lists/list_functions.h"

namespace snt::dip {

  enum class RequestType { Function,
                           Reference };

  class Environment {
  private:
  public:
    SourceList sources;
    UnitList units;
    NodeList nodes;
    HierarchyList hierarchy;
    BranchingList branching;
    FunctionList functions;
    Environment();
    std::string request_code(const std::string& source_name) const;
    val::BaseValue::PointerType request_value(const std::string& request, const RequestType rtype,
                                              const std::string& to_unit = "") const;
    BaseNode::NodeListType request_nodes(const std::string& request, const RequestType rtype) const;
  };

} // namespace snt::dip

#endif
