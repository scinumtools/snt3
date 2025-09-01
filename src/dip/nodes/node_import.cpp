#include "../environment.h"
#include "nodes.h"

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

  BaseNode::NodeListType ImportNode::parse(Environment& env) {
    NodeListType nodes;
    switch (value_origin) {
    case ValueOrigin::Function:
      nodes = env.request_nodes(value_raw.at(0), RequestType::Function);
      break;
    case ValueOrigin::Reference:
      nodes = env.request_nodes(value_raw.at(0), RequestType::Reference);
      break;
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
