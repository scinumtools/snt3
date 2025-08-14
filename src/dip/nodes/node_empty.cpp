#include "nodes.h"

namespace dip {

  BaseNode::PointerType EmptyNode::is_node(Parser& parser) {
    if (!parser.do_continue())
      return std::make_shared<EmptyNode>(parser);
    if (parser.part_comment())
      return std::make_shared<EmptyNode>(parser);
    return nullptr;
  }

} // namespace dip
