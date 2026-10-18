#ifndef DIP_NODE_LINK_H
#define DIP_NODE_LINK_H

#include <snt/dip/nodes/node_string.h>

namespace snt::dip {

    /** Identity-preserving node link. The target is stored, never copied. */
    class LinkNode : public StringNode {
      public:
        explicit LinkNode(Parser& parser) : StringNode(parser) {
            dtype = NodeDtype::Link;
            value_dtype = core::DataType::None;
            value_origin = ValueOrigin::Empty;
        }
        static ValueNode::PointerType is_node(Parser& parser);
        BaseNode::ListType parse(Environment& env) override;
        BaseNode::PointerType clone(const Path& path, std::optional<size_t> indent = std::nullopt) const override;
    };

} // namespace snt::dip

#endif
