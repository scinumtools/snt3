#ifndef DIP_FUNCTION_LIST_H
#define DIP_FUNCTION_LIST_H

#include "../../val.h"
#include "../nodes/node_value.h"

#import <map>

namespace snt::dip {

  class Environment;

  class FunctionList {
  public:
    typedef val::BaseValue::PointerType (*ValueFunctionType)(const Environment& env);
    typedef ValueNode::NodeListType (*TableFunctionType)(const Environment& env);

  private:
    std::map<std::string, ValueFunctionType> value_functions;
    std::map<std::string, TableFunctionType> table_functions;

  public:
    void append_value(const std::string& name, ValueFunctionType func);
    void append_table(const std::string& name, TableFunctionType func);
    ValueFunctionType get_value(const std::string& name) const;
    TableFunctionType get_table(const std::string& name) const;
  };

} // namespace snt::dip

#endif // DIP_FUNCTION_LIST_H
