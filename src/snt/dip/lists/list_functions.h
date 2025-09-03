#ifndef DIP_FUNCTION_LIST_H
#define DIP_FUNCTION_LIST_H

namespace snt::dip {

  class FunctionList {
  public:
    typedef val::BaseValue::PointerType (*ValueFunctionType)(const Environment& env);
    typedef BaseNode::NodeListType (*TableFunctionType)(const Environment& env);

  private:
    std::map<std::string, ValueFunctionType> value_functions;
    std::map<std::string, TableFunctionType> table_functions;

  public:
    void append_value(const std::string& name, ValueFunctionType func);
    void append_table(const std::string& name, TableFunctionType func);
    ValueFunctionType get_value(const std::string& name) const;
    TableFunctionType get_table(const std::string& name) const;
  };

}

#endif // DIP_FUNCTION_LIST_H
