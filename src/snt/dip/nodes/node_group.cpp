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
            std::string full_path =
                env.hierarchy.resolve_list_selectors(env.hierarchy.get_current_path(indent, path.name).name);
            env.hierarchy.set_collection(full_path, Path::Kind::Map, value_raw);
            return {};
        } else if (dtype_raw[1] == KEYWORD_LIST) {
            std::string full_path =
                env.hierarchy.resolve_list_selectors(env.hierarchy.get_current_path(indent, path.name).name);
            env.hierarchy.set_collection(full_path, Path::Kind::List, value_raw);
            return {};
        } else {
            BaseNode::ListType nodes;
            if (schemas.empty()) { // since we output the same node, we have to avoid infinite loops
                new_schema_applications.clear();
                // Add schemas from collection definitions
                std::string full_path =
                    env.hierarchy.resolve_list_selectors(env.hierarchy.get_current_path(indent, path.name, false).name);
                std::vector<std::string> previous_schemas;
                std::vector<std::string> inherited_schemas;
                if (env.hierarchy.has_collection(full_path)) {
                    const Collection& col = env.hierarchy.get_collection(full_path);
                    if (col.kind == Path::Kind::Group) {
                        // This concrete group already expanded these schemas; a later block only continues it.
                        previous_schemas = col.schemas;
                    } else {
                        inherited_schemas = col.schemas;
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
                }
                // Add all direct schemas
                for (const auto& schema : value_raw) {
                    if (std::find(previous_schemas.begin(), previous_schemas.end(), schema) != previous_schemas.end())
                        throw dip::SyntaxException(
                            "Duplicated schema",
                            "The schema `" + schema + "` is applied more than once to the same group.",
                            "Remove the repeated schema declaration from this group.",
                            __FILE__,
                            __LINE__,
                            line
                        );
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
                // A later declaration of a group can add schemas, but must not reapply earlier ones.
                const auto schemas_to_apply = schemas;
                if (!schemas_to_apply.empty()) {
                    for (const auto& schema_name : schemas_to_apply)
                        new_schema_applications.emplace_back(schema_name,
                            std::find(inherited_schemas.begin(), inherited_schemas.end(), schema_name) != inherited_schemas.end());
                    schemas.insert(schemas.begin(), previous_schemas.begin(), previous_schemas.end());
                    nodes.push_back(shared_from_this()); // Now we return the group node ... (hence the infinite loop)
                    for (const auto& schema_name : schemas_to_apply) {
                        EnvSchema schema = env.schemas.at(schema_name);
                        for (const auto& node : schema.nodes) { // ... and unwrap the schema nodes
                            BaseNode::PointerType node_new = node->clone(node->path, node->indent + indent);
                            node_new->schema_id = schema.id;
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

    BaseNode::PointerType GroupNode::clone(const Path& pth, std::optional<size_t> indent) const {
        std::shared_ptr<GroupNode> copy = std::make_shared<GroupNode>(*this);
        copy->path = pth;
        if (indent)
            copy->indent = indent.value();
        return copy;
    }

} // namespace snt::dip
