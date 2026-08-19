#include "../parsers.h"

#include <snt/dip/environment.h>
#include <snt/dip/nodes/node_deferred.h>

namespace snt::dip {

    BaseNode::ListType DeferredNode::parse(Environment& env) {
        return {};
    }

    DeferredNode::DeferredNode(BaseNode::PointerType nd) : node(nd), ValueNode(nd, NodeDtype::Deferred) {}

    val::BaseValue::PointerType DeferredNode::cast_scalar_value(const std::string& value_input) const {
        return nullptr;
    }

    val::BaseValue::PointerType DeferredNode::cast_array_value(
        const val::Array::StringType& value_inputs, const val::Array::ShapeType& shape
    ) const {
        return nullptr;
    }

} // namespace snt::dip
