#include "node_group.h"

namespace snt::dip {

  BaseNode::PointerType GroupNode::is_node(Parser& parser) {
    parser.part_comment();
    if (!parser.do_continue())
      return std::make_shared<GroupNode>(parser);
    return nullptr;
  }

} // namespace snt::dip
