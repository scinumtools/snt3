#include "node_modification.h"

#include "node_deferred.h"
#include "node_table.h"

#include <snt/dip/environment.h>

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

    BaseNode::ListType ModificationNode::parse(Environment& env) {
        // This is treating modifications of declared tables
        // First we search if there is already existing node with the same name
        std::string full_name = env.hierarchy.get_current_path(indent, path.name).name;
        for (size_t i = 0; i < env.nodes.size(); i++) {
            ValueNode::PointerType node = env.nodes.at(i);
            if (node->dtype != NodeDtype::Deferred)
                continue;
            // is it deferred?
            auto dnode = std::dynamic_pointer_cast<DeferredNode>(node);
            if (dnode && dnode->path.name == full_name) {
                // is it a table node?
                if (dnode->node->dtype == NodeDtype::Table) {
                    // then add value to the table node and parse it
                    auto tnode = std::dynamic_pointer_cast<TableNode>(dnode->node);
                    tnode->value_origin = value_origin;
                    tnode->value_raw = value_raw;
                    // return parsed nodes back to the node list
                    return tnode->parse(env);
                }
                break;
            }
        }
        return {};
    }

} // namespace snt::dip
