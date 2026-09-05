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
        using NodesFunctionType = ValueNode::ListType (*)(const Environment& env);

      private:
        std::map<std::string, ValueFunctionType> value_functions;
        std::map<std::string, NodesFunctionType> table_functions;

      public:
        void append_value(const std::string& name, ValueFunctionType func);
        void append_nodes(const std::string& name, NodesFunctionType func);
        ValueFunctionType get_value(const std::string& name) const;
        NodesFunctionType get_nodes(const std::string& name) const;
    };

} // namespace snt::dip

#endif // DIP_FUNCTION_LIST_H
