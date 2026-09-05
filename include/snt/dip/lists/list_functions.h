#ifndef DIP_FUNCTION_LIST_H
#define DIP_FUNCTION_LIST_H

#include <functional>
#include <map>
#include <snt/dip/nodes/node_value.h>

namespace snt::dip {

    class Environment;

    class FunctionList {
      public:
        using ValueFunctionType = std::function<val::BaseValue::PointerType(const Environment&)>;
        using DataFunctionType = std::function<ValueNodeData(const Environment&)>;
        using NodesFunctionType = std::function<ValueNode::ListType(const Environment&)>;

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
