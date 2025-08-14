#ifndef DIP_ENVIRONMENT_H
#define DIP_ENVIRONMENT_H

#include "lists/lists.h"

namespace dip {

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

} // namespace dip

#endif
