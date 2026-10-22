#include "node_import.h"

#include <array>
#include <fstream>
#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>

namespace snt::dip {

    BaseNode::PointerType ImportNode::is_node(Parser& parser) {
        if (parser.part_function()) {
            parser.part_comment();
            return std::make_shared<ImportNode>(parser);
        } else if (parser.part_reference()) {
            parser.part_comment();
            return std::make_shared<ImportNode>(parser);
        }
        return nullptr;
    }

    BaseNode::ListType ImportNode::parse(Environment& env) {
        ValueNode::ListType vnodes;
        switch (value_origin) {
        case ValueOrigin::Function: {
            vnodes = env.request_group(value_raw.at(0), RequestType::Function);
            break;
        }
        case ValueOrigin::Reference: {
            const std::string& request = value_raw.at(0);
            vnodes = env.request_group(request, RequestType::Reference, {}, path.name);
            if (!request.empty() && (request.back() == SIGN_SEPARATOR || request.back() == SIGN_QUERY)) {
                const size_t separator = request.find(SIGN_QUERY);
                const std::string source_name = request.substr(0, separator);
                std::string source_root = request.substr(separator + 1);
                if (!source_root.empty() && source_root.back() == SIGN_SEPARATOR)
                    source_root.pop_back();
                const EnvSource* named_source = source_name.empty() ? nullptr : &env.sources.at(source_name);
                const HierarchyList& source_hierarchy = named_source ? named_source->hierarchy : env.hierarchy;
                if (named_source)
                    env.schemas.import_referenced_from(
                        named_source->schemas, source_hierarchy.subtree_schemas(source_root)
                    );
                const std::string destination = env.hierarchy.get_current_path(indent, path.name).name;
                env.hierarchy.import_subtree(source_hierarchy, source_root, destination);
            }
            break;
        }
        default:
            throw dip::EnvironmentException(
                "Invalid import origin",
                "The import uses an unsupported value origin.",
                "Use a function or reference as the import source.",
                __FILE__,
                __LINE__,
                line
            );
        }
        // update node settings
        BaseNode::ListType nodes;
        for (const auto& vnode : vnodes) {
            vnode->indent += indent;
            if (value_origin == ValueOrigin::Function && !path.name.empty())
                vnode->path = Path(path.name + std::string(1, SIGN_SEPARATOR) + vnode->path.name);
            size_t size = vnode->value_raw.size();
            vnode->value_shape = {size};
            if (vnode->dimension.empty() && size > 1) {
                vnode->dimension = {{size, size}};
            }
            nodes.push_back(vnode);
        }
        return nodes;
    }

} // namespace snt::dip
