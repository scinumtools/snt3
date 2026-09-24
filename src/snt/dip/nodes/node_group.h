#ifndef DIP_NODE_GROUP_H
#define DIP_NODE_GROUP_H

#include <snt/dip/nodes/node_base.h>

namespace snt::dip {

    class GroupNode : public BaseNode {
      public:
        static BaseNode::PointerType is_node(Parser& parser);
        GroupNode(Parser& parser) : BaseNode(parser, NodeDtype::Group) {};
        BaseNode::ListType parse(Environment& env) override;
        BaseNode::PointerType clone(const Path& pth, std::optional<size_t> indent = std::nullopt) const override;
    };

} // namespace snt::dip

#endif // DIP_NODE_GROUP_H
