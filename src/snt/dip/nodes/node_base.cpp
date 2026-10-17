#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_base.h>
#include <sstream>

namespace snt::dip {

    BaseNode::ListType BaseNode::parse(Environment& env) {
        return {};
    }

    bool BaseNode::set_property(PropertyType property, val::Array::StringType& values, std::string& units) {
        (void)units;
        if (property == PropertyType::MetadataConstraint && values.size() == 2) {
            const auto constrained_property = ValueMetadata::property_type(values[0]);
            if (!constrained_property || (values[1] != "required" && values[1] != "nonempty"))
                return false;
            metadata_constraints.push_back({
                *constrained_property,
                values[1] == "required" ? MetadataRule::Required : MetadataRule::Nonempty,
                {},
                line.source.name,
                line.source.line_number
            });
            return true;
        }
        if (metadata.set_property(property, values))
            return true;
        throw dip::SyntaxException(
            "Unsupported node property",
            "This node type does not implement property handling.",
            "Use a node type that supports properties.",
            __FILE__,
            __LINE__,
            line
        );
        return false;
    }

    BaseNode::PointerType BaseNode::clone(const Path& pth, std::optional<size_t> indent) const {
        std::shared_ptr<BaseNode> copy = std::make_shared<BaseNode>(*this);
        copy->path = Path(pth.name);
        if (indent)
            copy->indent = indent.value();
        return copy;
    }

    std::string BaseNode::to_string(const core::StringFormatType& format) const {
        throw dip::EnvironmentException(
            "Unsupported string representation",
            "This node type does not implement `to_string()`.",
            "Use a node type that provides a string representation.",
            __FILE__,
            __LINE__,
            line
        );
        return "";
    }

} // namespace snt::dip
