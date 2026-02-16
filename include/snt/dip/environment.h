#ifndef DIP_ENVIRONMENT_H
#define DIP_ENVIRONMENT_H

#include <snt/dip/lists/list_branching.h>
#include <snt/dip/lists/list_functions.h>
#include <snt/dip/lists/list_hierarchy.h>
#include <snt/dip/lists/list_node.h>
#include <snt/dip/lists/list_source.h>
#include <snt/dip/lists/list_unit.h>
#include "nodes/node_value.h"

namespace snt::dip {

  /**
   * @enum RequestType
   * @brief Type of an environment request
   */
  enum class RequestType {
    Reference,   ///< Search in the node list
    Function     ///< Search in the function list
  };

  /**
   * @class Environment
   * @brief Object of this class holds the whole DIP parsing environment 
   */
  class Environment {
  private:
  public:
    NodeList<ValueNode> nodes; ///< List of parsed nodes
    HierarchyList hierarchy;   ///< List of node hierarchy (parent nodes)
    BranchingList branching;   ///< List of code branching (case, else)
    SourceList sources;        ///< List of code sources
    UnitList units;            ///< List of custom units
    FunctionList functions;    ///< List of functions

    /**
     * @brief Constructor of the Environment class
     */
    Environment();

    /**
     * @brief Get a source code
     *
     * @param source_name Name of a source
     * @return Source code
     */
    std::string request_code(const std::string& source_name) const;

    /**
     * @brief Get value from a reference or a function based on a request expression
     *
     * @param request Request expression
     * @param rtype Request type: reference, or function
     * @param to_unit Request values with a specific unit
     * @return Selected ArrayValue object (in specified units)
     */
    val::BaseValue::PointerType request_value(const std::string& request,
					      const RequestType rtype = RequestType::Reference,
                                              const std::string& to_unit = "") const;

    /**
     * @brief Get list of nodes from a reference or a function based on a request expression
     *
     * @param request Request expression
     * @param rtype Request type: reference, or function
     * @return List of selected nodes
     */
    ValueNode::ListType request_nodes(const std::string& request,
				      const RequestType rtype = RequestType::Reference) const;

    /**
     * @brief Get parsed node value at the specific index
     *
     * @param index Index of a node
     * @return ArrayValue of a selected node
     */
    val::BaseValue::PointerType get_value(size_t index) const;
  };

} // namespace snt::dip

#endif
