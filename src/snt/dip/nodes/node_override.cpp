#include "node_override.h"

namespace snt::dip {

    BaseNode::PointerType OverrideNode::is_node(Parser& parser) {
        if (parser.kwd_override()) {
            parser.part_comment();
            return std::make_shared<OverrideNode>(parser);
        }
        return nullptr;
    }

} // namespace snt::dip
