#include "node_import.h"

#include <snt/dip/environment.h>

#include <array>
#include <fstream>

namespace snt::dip {

  BaseNode::PointerType ImportNode::is_node(Parser& parser) {
    if (parser.part_function()) {
      parser.part_comment();
      return std::make_shared<ImportNode>(parser);
    } else if (parser.part_reference()) {
      parser.part_comment();
      return std::make_shared<ImportNode>(parser);
    }
    return nullptr;
  }

  BaseNode::ListType ImportNode::parse(Environment& env) {
    BaseNode::ListType nodes;
    switch (value_origin) {
    case ValueOrigin::Function: {
      ValueNode::ListType vnodes = env.request_nodes(value_raw.at(0), RequestType::Function);
      for (const auto& node : vnodes)
        nodes.push_back(node);
      break;
    }
    case ValueOrigin::Reference: {
      ValueNode::ListType vnodes = env.request_nodes(value_raw.at(0), RequestType::Reference);
      for (const auto& node : vnodes)
        nodes.push_back(node);
      break;
    }
    default:
      throw std::runtime_error("Import nodes could not be parsed: " + line.code);
    }
    // update node settings
    for (const auto& node : nodes) {
      size_t size = node->value_raw.size();
      node->indent += indent;
      if (!name.empty())
        node->name = name + std::string(1, SIGN_SEPARATOR) + node->name;
      node->value_shape = {size};
      if (node->dimension.empty())
        node->dimension = {{size, size}};
    }
    return nodes;
  }

} // namespace snt::dip
