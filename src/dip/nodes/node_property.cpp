#include "../environment.h"
#include "nodes.h"

namespace snt::dip {

  BaseNode::PointerType PropertyNode::is_node(Parser& parser) {
    PropertyType ptype = PropertyType::None;
    // determine property type
    if (parser.part_equal(false))
      ptype = PropertyType::Options;
    else
      parser.kwd_property(ptype);
    // parse rest of the parts
    if (ptype != PropertyType::None) {
      parser.part_value();
      parser.part_units();
      parser.part_comment();
      return std::make_shared<PropertyNode>(parser, ptype);
    }
    return nullptr;
  }

} // namespace snt::dip
