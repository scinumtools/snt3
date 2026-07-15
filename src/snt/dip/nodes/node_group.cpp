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
        // EnvSource senv = parse_source(value_raw.at(0), value_raw.at(1), line.source);
        // env.sources.append(value_raw.at(0), senv);
        if (value_raw.size() > 0) {
            EnvSchema schema = env.schemas.at(value_raw.at(0));
            BaseNode::ListType nodes;
            for (const auto& vnode : schema.nodes) {
                ValueNode::PointerType vnode_new = std::dynamic_pointer_cast<ValueNode>(vnode->clone(vnode->path));
                vnode_new->indent += indent;
                nodes.push_back(vnode_new);
            }
            return nodes;
        } else {
            return {};
        }
    }

} // namespace snt::dip
