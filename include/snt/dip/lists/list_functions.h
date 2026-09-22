#ifndef DIP_FUNCTION_LIST_H
#define DIP_FUNCTION_LIST_H

#include <functional>
#include <map>
#include <snt/dip/nodes/node_value.h>
#include <vector>

namespace snt::dip {

    class Environment;

    enum class FunctionKind { Value, Nodes };

    /** Traceable registered DIPL function. */
    struct EnvFunction {
        std::string id;   ///< Internal trace identifier, e.g. DIP0_FUNCTION0.
        std::string name; ///< Public function name used by DIPL.
        FunctionKind kind;
    };

    /** Registry of functions callable from DIPL expressions. */
    class FunctionList {
      public:
        using DataFunctionType = std::function<ValueNodeData(const Environment&)>;
        using NodesFunctionType = std::function<ValueNode::ListType(const Environment&)>;

      private:
        std::map<std::string, DataFunctionType> data_functions;
        std::map<std::string, NodesFunctionType> nodes_functions;
        std::map<std::string, EnvFunction> data_function_info;
        std::map<std::string, EnvFunction> nodes_function_info;
        std::map<std::string, size_t> id_counters;

      public:
        /** Register a value function under a public name and trace parent identifier. */
        void append_value(const std::string& name, DataFunctionType func, const std::string& parent_id = {});
        /** Register a node-producing function under a public name and trace parent identifier. */
        void append_nodes(const std::string& name, NodesFunctionType func, const std::string& parent_id = {});
        /** Return the registered value-producing function.
         * @param name Function name to look up.
         */
        DataFunctionType get_value(const std::string& name) const;
        NodesFunctionType get_nodes(const std::string& name) const;
        /** Return trace information for a registered value function. */
        const EnvFunction& get_value_info(const std::string& name) const;
        /** Return trace information for a registered node-producing function. */
        const EnvFunction& get_nodes_info(const std::string& name) const;
        /** Return trace information for all registered functions, ordered by name and kind. */
        std::vector<EnvFunction> entries() const;
    };

} // namespace snt::dip

#endif // DIP_FUNCTION_LIST_H
