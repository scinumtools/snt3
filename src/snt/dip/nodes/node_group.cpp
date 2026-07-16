#include "node_group.h"

#include <snt/dip/environment.h>

namespace snt::dip {

    BaseNode::PointerType GroupNode::is_node(Parser& parser) {
        parser.part_schema();
        parser.part_comment();
        if (!parser.do_continue())
            return std::make_shared<GroupNode>(parser);
        return nullptr;
    }

    BaseNode::ListType GroupNode::parse(Environment& env) {
        // TODO: implement import of a source
        // TODO: implement injection of a source file
        // TODO: implement injection a text file
        if (value_raw.size() > 0) {
            EnvSchema schema = env.schemas.at(value_raw.at(0));
            value_raw.clear(); // We need to clear the value, because it would cause an infinite loop in the queue
            BaseNode::ListType nodes;
            nodes.push_back(shared_from_this()); // Now we return the group node ...
            for (const auto& node : schema.nodes) {
                BaseNode::PointerType node_new = node->clone(node->path, node->indent + indent);
                nodes.push_back(node_new); // ... and unwrap the schema nodes
            }
            return nodes;
        } else {
            return {};
        }
    }

} // namespace snt::dip
