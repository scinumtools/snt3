#include "node_link.h"

#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>

namespace snt::dip {

    ValueNode::PointerType LinkNode::is_node(Parser& parser) {
        if (parser.dtype_raw[1] != KEYWORD_LINK)
            return nullptr;
        if (parser.part_equal(false) &&
            (!parser.part_reference() || parser.value_origin != ValueOrigin::Reference))
            throw dip::SyntaxException(
                "Invalid link target",
                "A link requires an absolute node reference such as `{?catalog.items[key]}`.",
                "Use `link = {?path}` or `link = {source?path}` and avoid a trailing subtree selector.",
                __FILE__, __LINE__, parser.line
            );
        parser.part_units();
        if (!parser.units_raw.empty())
            throw dip::SyntaxException(
                "Invalid link units", "A link cannot have units.", "Remove the unit annotation from the link.",
                __FILE__, __LINE__, parser.line
            );
        parser.part_comment();
        return std::make_shared<LinkNode>(parser);
    }

    BaseNode::ListType LinkNode::parse(Environment&) {
        if (value_raw.empty())
            return {};
        const std::string& request = value_raw.at(0);
        const size_t separator = request.find(SIGN_QUERY);
        if (separator == std::string::npos || separator + 1 == request.size() || request.back() == SIGN_SEPARATOR)
            throw dip::SyntaxException(
                "Invalid link target", "A link must identify one node path.",
                "Use `{?path}` or `{source?path}` without a trailing dot.", __FILE__, __LINE__, line
            );
        link_source = request.substr(0, separator);
        link_target = request.substr(separator + 1);
        value_origin = ValueOrigin::Empty;
        return {};
    }

    void LinkNode::modify_value(const BaseNode::PointerType& node, Environment& env) {
        if (!link_target.empty())
            throw dip::SyntaxException(
                "Read-only link", "The link node `" + path.name + "` already has a target.",
                "Modify the canonical target path instead.", __FILE__, __LINE__, node->line
            );
        // Nested schema expansion may repeat an unassigned member declaration.
        if (node->dtype == NodeDtype::Link && node->value_raw.empty()) {
            metadata.merge_explicit(node->metadata);
            return;
        }
        if (node->dtype != NodeDtype::Modification || node->value_origin != ValueOrigin::Reference ||
            node->value_raw.size() != 1 || !node->units_raw.empty())
            throw dip::SyntaxException(
                "Invalid link target", "An unassigned link requires one absolute node reference without units.",
                "Assign `{?path}` or `{source?path}` to the declared link.", __FILE__, __LINE__, node->line
            );
        const std::string& request = node->value_raw.front();
        const size_t separator = request.find(SIGN_QUERY);
        if (separator == std::string::npos || separator + 1 == request.size() || request.back() == SIGN_SEPARATOR)
            throw dip::SyntaxException(
                "Invalid link target", "A link must identify one node path.",
                "Use `{?path}` or `{source?path}` without a trailing dot.", __FILE__, __LINE__, node->line
            );
        link_source = request.substr(0, separator);
        link_target = request.substr(separator + 1);
        value_raw = node->value_raw;
        metadata.merge_explicit(node->metadata);
        modification_lines.push_back(node->line);
        if (env.block_input_recording())
            env.erase_block_input(path.name);
    }

    BaseNode::PointerType LinkNode::clone(const Path& path, std::optional<size_t> indent) const {
        auto copy = std::make_shared<LinkNode>(*this);
        copy->path = path;
        if (indent)
            copy->indent = *indent;
        return copy;
    }

} // namespace snt::dip
