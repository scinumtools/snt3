#ifndef DIP_NODE_MODIFICATION_H
#define DIP_NODE_MODIFICATION_H

#include "node_base.h"

namespace snt::dip {

  class ModificationNode : public BaseNode {
  public:
    static BaseNode::PointerType is_node(Parser& parser);
    ModificationNode(Parser& parser) : BaseNode(parser, NodeDtype::Modification) {};
  };

} // namespace snt::dip

#endif // DIP_NODE_MODIFICATION_H
