#include <snt/dip/nodes/node_base.h>
#include <sstream>

namespace snt::dip {

    BaseNode::ListType BaseNode::parse(Environment& env) {
        return {};
    }

    bool BaseNode::set_property(PropertyType property, val::Array::StringType& values, std::string& units) {
        throw std::runtime_error("Properties are not implemented for this node: " + line.code);
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
        throw std::runtime_error("String representation of a node is not implemented: " + line.code);
        return "";
    }

} // namespace snt::dip
