#ifndef DIP_FUNCTION_LIST_H
#define DIP_FUNCTION_LIST_H

#include <map>
#include <snt/dip/nodes/node_value.h>

namespace snt::dip {

    class Environment;

    class FunctionList {
      public:
        using ValueFunctionType = val::BaseValue::PointerType (*)(const Environment& env);
        using DataFunctionType = ValueNodeData (*)(const Environment& env);
        using TableFunctionType = ValueNode::ListType (*)(const Environment& env);

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
