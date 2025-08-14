#include <sstream>

#include "nodes.h"

namespace dip {

  BaseNode::BaseNode(Parser& parser, const NodeDtype dt) : dtype(dt) {
    line = parser.line;
    indent = parser.indent;
    name = parser.name;
    dtype_raw = parser.dtype_raw;
    dimension = parser.dimension;
    value_raw = parser.value_raw;
    value_shape = parser.value_shape;
    value_origin = parser.value_origin;
    value_slice = parser.value_slice;
    units_raw = parser.units_raw;
  }

  BaseNode::NodeListType BaseNode::parse(Environment& env) {
    return {};
  }

  bool BaseNode::set_property(PropertyType property, Array::StringType& values, std::string& units) {
    throw std::runtime_error("Properties are not implemented for this node: " + line.code);
    return false;
  }

} // namespace dip
