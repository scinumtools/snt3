#include "environment.h"

#include "nodes/node_quantity.h"
#include "nodes/node_value.h"

namespace snt::dip {

  inline std::tuple<std::string, std::string> parse_request(const std::string& request) {
    size_t pos = request.find(SIGN_QUERY);
    if (pos == std::string::npos)
      throw std::runtime_error("Environment request must contain a question mark symbol: " +
                               request);
    else
      return {request.substr(0, pos), request.substr(pos + 1)};
  }

  Environment::Environment() = default;

  std::string Environment::request_code(const std::string& source_name) const {
    return sources.at(source_name).code;
  }

  val::BaseValue::PointerType Environment::request_value(const std::string& request,
                                                         const RequestType rtype,
                                                         const std::string& to_unit) const {
    val::BaseValue::PointerType new_value = nullptr;
    switch (rtype) {
    case RequestType::Function: {
      FunctionList::ValueFunctionType func = functions.get_value(request);
      new_value = func(*this);
      break;
    }
    case RequestType::Reference: {
      auto [source_name, node_path] = parse_request(request);
      const NodeList& node_pool = (source_name.empty()) ? nodes : sources.at(source_name).nodes;
      for (size_t i = 0; i < node_pool.size(); i++) {
        BaseNode::PointerType node = node_pool.at(i);
        ValueNode::PointerType vnode = std::dynamic_pointer_cast<ValueNode>(node);
        if (vnode and vnode->name == node_path) {
          new_value = vnode->value->clone();
          QuantityNode::PointerType qnode = std::dynamic_pointer_cast<QuantityNode>(node);
          if (qnode && to_unit != KEYWORD_NONE) {
            // NOTE: If unit conversion is not required, the to_unit should be set to "none".
            //       This is usefull if we want to simply get a reference node as it is.
            if (qnode->units == nullptr and !to_unit.empty())
              throw std::runtime_error(
                  "Trying to convert nondimensional quantity into '" + qnode->units_raw +
                  "': " + qnode->line.code);
            else if (qnode->units != nullptr and to_unit.empty())
              throw std::runtime_error(
                  "Trying to convert '" + qnode->units_raw +
                  "' into a nondimensional quantity: " + qnode->line.code);
            else if (qnode->units != nullptr) {
              puq::Quantity quantity = std::move(new_value) * (*qnode->units);
              quantity = quantity.convert(to_unit);
              new_value = std::move(quantity.value.magnitude.value);
            }
          }
        }
      }
      break;
    }
    default:
      throw std::runtime_error("Unrecognized environment request type");
    }
    if (new_value == nullptr)
      throw std::runtime_error("Value environment request returns an empty pointer: " + request);
    return std::move(new_value);
  }

  BaseNode::NodeListType Environment::request_nodes(const std::string& request,
                                                    const RequestType rtype) const {
    BaseNode::NodeListType new_nodes;
    switch (rtype) {
    case RequestType::Function: {
      FunctionList::TableFunctionType func = functions.get_table(request);
      new_nodes = func(*this);
      break;
    }
    case RequestType::Reference: {
      auto [source_name, node_path] = parse_request(request);
      std::string node_path_child = (!node_path.empty()) ? node_path + std::string(1, SIGN_SEPARATOR) : node_path;
      const NodeList& node_pool = (source_name.empty()) ? nodes : sources.at(source_name).nodes;
      size_t p = node_pool.size(); // parent node index
      for (size_t i = 0; i < node_pool.size(); i++) {
        ValueNode::PointerType vnode = std::dynamic_pointer_cast<ValueNode>(node_pool.at(i));
        if (vnode and vnode->name.rfind(node_path_child, 0) == 0 and
            vnode->name.size() > node_path_child.size()) {
          std::string new_name = vnode->name.substr(node_path_child.size(), vnode->name.size());
          new_nodes.push_back(vnode->clone(new_name));
        } else if (vnode and vnode->name == node_path) {
          p = i;
        }
      }
      // if node does not have childs, but exists, return it
      if (new_nodes.empty() && p < node_pool.size()) {
        ValueNode::PointerType vnode = std::dynamic_pointer_cast<ValueNode>(node_pool.at(p));
        size_t pos = node_path.find_last_of('.');
        if (pos != std::string::npos) {
          std::string new_name = vnode->name.substr(pos, node_path.size());
          new_nodes.push_back(vnode->clone(new_name));
        } else {
          new_nodes.push_back(vnode->clone(node_path));
        }
      }
      break;
    }
    default:
      throw std::runtime_error("Unrecognized environment request type");
    }
    if (new_nodes.empty())
      throw std::runtime_error("Node environment request returns an empty node list: " + request);
    return new_nodes;
  }

} // namespace snt::dip
