#include <stdexcept>

#include "../environment.h"
#include "../parsers.h"
#include "nodes.h"

namespace dip {

  BaseNode::PointerType TableNode::is_node(Parser& parser) {
    if (parser.dtype_raw[1] == "table") {
      parser.part_dimension();
      if (parser.part_equal(false))
        parser.part_value();
      parser.part_units();
      parser.part_comment();
      return std::make_shared<TableNode>(parser);
    }
    return nullptr;
  }

  inline BaseNode::NodeListType parse_nodes(const std::string& value_raw,
                                            const std::string& source_name, const char delimiter) {
    std::queue<Line> lines;
    parse_lines(lines, value_raw, source_name);
    return parse_table_nodes(lines, delimiter);
  }

  BaseNode::NodeListType TableNode::parse(Environment& env) {
    std::string source_name = line.source.name + "_" + std::string(TABLE_SOURCE);
    NodeListType nodes;
    switch (value_origin) {
    case ValueOrigin::Function:
      nodes = env.request_nodes(value_raw.at(0), RequestType::Function);
      break;
    case ValueOrigin::Reference:
      nodes = env.request_nodes(value_raw.at(0), RequestType::Reference);
      break;
    case ValueOrigin::ReferenceRaw:
      nodes = parse_nodes(env.request_code(value_raw.at(0)), source_name, delimiter);
      break;
    case ValueOrigin::String:
      nodes = parse_nodes(value_raw.at(0), source_name, delimiter);
      break;
    default:
      throw std::runtime_error("Table nodes could not be parsed: " + line.code);
    }
    // update node settings
    for (auto node : nodes) {
      int size = node->value_raw.size();
      node->indent += indent;
      node->name = name + std::string(1, SIGN_SEPARATOR) + node->name;
      node->value_shape = {size};
      if (node->dimension.empty())
        node->dimension = {{size, size}};
    }
    return nodes;
  }

  bool TableNode::set_property(PropertyType property, Array::StringType& values, std::string& units) {
    std::string delimiter_raw = values.at(0);
    if (property == PropertyType::Delimiter and !delimiter_raw.empty()) {
      delimiter = delimiter_raw[0];
      return true;
    }
    return false;
  }

} // namespace dip
