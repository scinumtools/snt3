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
            for (const auto& node : schema.nodes) {
                BaseNode::PointerType node_new = node->clone(node->path);
                node_new->indent = indent + INDENT_STEP;
                nodes.push_back(node_new);
            }
            return nodes;
        } else {
            return {};
        }
    }

} // namespace snt::dip
