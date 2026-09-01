#include "node_group.h"

#include "node_deferred.h"

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
            if (schemas.empty()) { // since we output the same node, we have to avoid infinite loops
                // Add schemas from collection definitions
                std::string full_path = env.hierarchy.get_current_path(indent, path.name, false).name;
                if (env.hierarchy.has_collection(full_path)) {
                    Collection col = env.hierarchy.get_collection(full_path);
                    for (const auto& schema : col.schemas) {
                        if (std::find(schemas.begin(), schemas.end(), schema) == schemas.end()) {
                            schemas.push_back(schema);
                        } else {
                            throw dip::SyntaxException(
                                "Duplicated schema",
                                "The schema `" + schema + "` is applied twice to the same item.",
                                "The schema was declared more than once on the same collection. "
                                "Remove one of the declarations.",
                                __FILE__,
                                __LINE__,
                                line
                            );
                        }
                    }
                }
                // Add all direct schemas
                for (const auto& schema : value_raw) {
                    if (std::find(schemas.begin(), schemas.end(), schema) != schemas.end()) {
                        throw dip::SyntaxException(
                            "Duplicated schema",
                            "The schema `" + schema + "` is applied more than once to the same item.",
                            "The schema was probably declared both in the collection definition and on the item. "
                            "Remove one of the duplicate schema declarations.",
                            __FILE__,
                            __LINE__,
                            line
                        );
                    }

                    schemas.push_back(schema);
                }
                // Apply all schemas
                if (!schemas.empty()) {
                    nodes.push_back(shared_from_this()); // Now we return the group node ... (hence the infinite loop)
                    for (const auto& schema_name : schemas) {
                        EnvSchema schema = env.schemas.at(schema_name);
                        for (const auto& node : schema.nodes) { // ... and unwrap the schema nodes
                            BaseNode::PointerType node_new = node->clone(node->path, node->indent + indent);
                            // if schema contains table nodes, we defer it
                            if (node_new->dtype == NodeDtype::Table)
                                node_new = std::make_shared<DeferredNode>(node_new);
                            nodes.push_back(node_new);
                        }
                    }
                }
            }
            return nodes;
        }
    }

} // namespace snt::dip
