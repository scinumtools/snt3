#ifndef DIP_ENVIRONMENT_H
#define DIP_ENVIRONMENT_H

#include "nodes/node_value.h"

#include <snt/dip/lists/list_branching.h>
#include <snt/dip/lists/list_functions.h>
#include <snt/dip/lists/list_hierarchy.h>
#include <snt/dip/lists/list_node.h>
#include <snt/dip/lists/list_schema.h>
#include <snt/dip/lists/list_source.h>
#include <snt/dip/lists/list_unit.h>

namespace snt::dip {

    class Cursor; ///< Forward declaring

    /**
     * Type of an environment request
     */
    enum class RequestType {
        Reference, ///< Search in the node list
        Function   ///< Search in the function list
    };

    /**
     * Object of this class holds the whole DIP parsing environment
     */
    class Environment {
      private:
      public:
        NodeList<ValueNode> nodes; ///< List of parsed nodes
        HierarchyList hierarchy;   ///< List of node hierarchy (parent nodes)
        BranchingList branching;   ///< List of code branching (case, else)
        SourceList sources;        ///< List of code sources
        UnitList units;            ///< List of custom units
        SchemaList schemas;        ///< List of schemas
        FunctionList functions;    ///< List of functions

        /**
         * Constructor of the Environment class
         */
        Environment();

        /**
         * Get a source code
         *
         * @param source_name Name of a source
         * @return Source code
         */
        std::string request_code(const std::string& source_name) const;

        /**
         * Get node data (value + units) from a reference or a function based on a request expression
         *
         * @param request Request expression
         * @param rtype Request type: reference, or function
         * @return Selected value node data
         */
        ValueNodeData request_node_data(
            const std::string& request, const RequestType rtype = RequestType::Reference
        ) const;

        /**
         * Get value from a reference or a function based on a request expression
         *
         * @param request Request expression
         * @param rtype Request type: reference, or function
         * @param to_unit Request values with a specific unit
         * @return Selected ArrayValue object (in specified units)
         */
        val::BaseValue::PointerType request_value(
            const std::string& request,
            const RequestType rtype = RequestType::Reference,
            const std::string& to_unit = ""
        ) const;

        /**
         * Get group of nodes from a reference or a function based on a request expression
         *
         * @param request Request expression
         * @param rtype Request type: reference, or function
         * @param tags List of tags that filter selected set
         * @return Group of selected nodes
         */
        ValueNode::ListType request_group(
            const std::string& request,
            const RequestType rtype = RequestType::Reference,
            const std::vector<std::string>& tags = {}
        ) const;

        /**
         * Get a keyed collection of  nodes from a reference or a function based on a request expression
         *
         * @param request Request expression
         * @param rtype Request type: reference, or function
         * @param tags List of tags that filter selected set
         * @return Keyed collection of nodes
         */
        std::unordered_map<std::string, ValueNode::ListType> request_map(
            const std::string& request,
            const RequestType rtype = RequestType::Reference,
            const std::vector<std::string>& tags = {}
        ) const;

        /**
         * Get a indexed collection of nodes from a reference or a function based on a request expression
         *
         * @param request Request expression
         * @param rtype Request type: reference, or function
         * @param tags List of tags that filter selected set
         * @return Indexed collection of nodes
         */
        std::vector<ValueNode::ListType> request_list(
            const std::string& request,
            const RequestType rtype = RequestType::Reference,
            const std::vector<std::string>& tags = {}
        ) const;

        /**
         * Get parsed node value at the specific index
         *
         * @param index Index of a node
         * @return ArrayValue of a selected node
         */
        val::BaseValue::PointerType get_value(size_t index) const;

        /**
         * Represent environment as a string
         *
         * @return String representation of an environment
         */
        const std::string to_string() const;

        /**
         * Get cursor from a fully qualified path
         *
         * @return Cursor at the given path
         */
        Cursor operator[](std::string_view path) const;
    };

} // namespace snt::dip

#endif
