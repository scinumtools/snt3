#include "node_group.h"

#include <snt/dip/environment.h>

namespace snt::dip {

    BaseNode::PointerType GroupNode::is_node(Parser& parser) {
        parser.part_schema();
        parser.part_comment();
        parser.part_trim();
        if (!parser.do_continue())
            return std::make_shared<GroupNode>(parser);
        return nullptr;
    }

    BaseNode::ListType GroupNode::parse(Environment& env) {
        // TODO: implement import of a source
        // TODO: implement injection of a source file
        // TODO: implement injection a text file
        if (dtype_raw[1] == KEYWORD_MAP) {
            std::string full_path = env.hierarchy.get_current_path(indent, path.name).name;
            env.hierarchy.set_collection(full_path, Path::Kind::Map, value_raw);
            return {};
        } else if (dtype_raw[1] == KEYWORD_LIST) {
            std::string full_path = env.hierarchy.get_current_path(indent, path.name).name;
            env.hierarchy.set_collection(full_path, Path::Kind::List, value_raw);
            return {};
        } else {
            BaseNode::ListType nodes;
            if (schemas.empty()) { // since we output the same node, we have to  avoid infinite loop
                // Add schemas from collection definitions
                std::string full_path = env.hierarchy.get_current_path(indent, path.name, false).name;
                if (env.hierarchy.has_collection(full_path)) {
                    Collection col = env.hierarchy.get_collection(full_path);
                    if (!col.schemas.empty())
                        std::copy(col.schemas.begin(), col.schemas.end(), std::back_inserter(schemas));
                }
                // Add all direct schemas
                if (value_raw.size() > 0) {
                    std::copy(value_raw.begin(), value_raw.end(), std::back_inserter(schemas));
                }
                // Apply all schemas
                if (!schemas.empty()) {
                    nodes.push_back(shared_from_this()); // Now we return the group node ... (hence infinite loop)
                    for (const auto& schema_name : schemas) {
                        EnvSchema schema = env.schemas.at(schema_name);
                        for (const auto& node : schema.nodes) { // ... and unwrap the schema nodes
                            BaseNode::PointerType node_new = node->clone(node->path, node->indent + indent);
                            nodes.push_back(node_new);
                        }
                    }
                }
            }
            return nodes;
        }
    }

} // namespace snt::dip
