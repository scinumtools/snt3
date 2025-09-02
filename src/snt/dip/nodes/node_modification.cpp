#include "nodes.h"

namespace snt::dip {

  BaseNode::PointerType ModificationNode::is_node(Parser& parser) {
    if (parser.part_equal(false)) {
      parser.part_value();
      parser.part_units();
      parser.part_comment();
      return std::make_shared<ModificationNode>(parser);
    }
    return nullptr;
  }

} // namespace snt::dip
