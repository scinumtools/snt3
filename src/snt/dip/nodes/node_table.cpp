#include "node_table.h"

#include "../parsers.h"
#include "node_deferred.h"

#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <stdexcept>

namespace snt::dip {

    BaseNode::PointerType TableNode::is_node(Parser& parser) {
        if (parser.dtype_raw[1] == "table") {
            parser.part_dimension();
            if (parser.part_equal(false))
                parser.part_value();
            parser.part_units();
            parser.part_comment();
            return std::make_shared<TableNode>(parser);
        }
        return nullptr;
    }

    inline BaseNode::ListType parse_nodes(
        const std::string& value_raw, const std::string& source_name, const char delimiter
    ) {
        std::queue<Line> lines;
        parse_lines(lines, value_raw, source_name);
        return parse_table_nodes(lines, delimiter);
    }

    BaseNode::ListType TableNode::parse(Environment& env) {
        std::string source_name = line.source.name + "_" + std::string(TABLE_SOURCE);
        BaseNode::ListType nodes;
        switch (value_origin) {
        case ValueOrigin::Function: {
            ValueNode::ListType vnodes = env.request_group(value_raw.at(0), RequestType::Function);
            for (const auto& node : vnodes)
                nodes.push_back(node);
            break;
        }
        case ValueOrigin::Reference: {
            ValueNode::ListType vnodes = env.request_group(value_raw.at(0), RequestType::Reference);
            for (const auto& node : vnodes)
                nodes.push_back(node);
            break;
        }
        case ValueOrigin::ReferenceRaw:
            nodes = parse_nodes(env.request_code(value_raw.at(0)), source_name, delimiter);
            break;
        case ValueOrigin::String:
            nodes = parse_nodes(value_raw.at(0), source_name, delimiter);
            break;
        case ValueOrigin::Empty: {
            // if value is empty, we have to defer this node until its value will be defined
            DeferredNode::PointerType node = std::make_shared<DeferredNode>(shared_from_this());
            nodes.push_back(node);
            return nodes;
        }
        default:
            throw dip::SyntaxException(
                "Invalid table node origin",
                "A table node must be defined using a function, reference, string, or empty value origin.",
                "The node has an unsupported value origin.",
                "Check the node value and use a supported value origin.",
                __FILE__,
                __LINE__,
                line
            );
        }
        // we have to check if a deferred table node is already in the node list
        std::string full_name = env.hierarchy.get_current_path(indent, path.name).name;
        for (size_t i = 0; i < env.nodes.size(); i++) {
            auto node = env.nodes.at(i);
            if (node->dtype != NodeDtype::Deferred)
                continue;
            // if yes, erase it
            auto dnode = std::dynamic_pointer_cast<DeferredNode>(node);
            if (dnode && dnode->path.name == full_name) {
                env.nodes.erase(i);
                break;
            }
        }
        // update node settings
        for (const auto& node : nodes) {
            size_t size = node->value_raw.size();
            node->indent += indent;
            node->path.name = path.name + std::string(1, SIGN_SEPARATOR) + node->path.name;
            node->path.collections.insert(
                node->path.collections.begin(), path.collections.begin(), path.collections.end()
            );
            node->value_shape = {size};
            if (node->dimension.empty())
                node->dimension = {{size, size}};
        }
        return nodes;
    }

    bool TableNode::set_property(PropertyType property, val::Array::StringType& values, std::string& units) {
        const std::string& delimiter_raw = values.at(0);
        if (property == PropertyType::Delimiter && !delimiter_raw.empty()) {
            delimiter = delimiter_raw[0];
            return true;
        }
        return false;
    }

} // namespace snt::dip
