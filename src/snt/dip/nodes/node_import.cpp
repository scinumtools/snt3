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
            vnodes = env.request_group(value_raw.at(0), RequestType::Reference);
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
            if (!path.name.empty())
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
