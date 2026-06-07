#ifndef DIP_NODE_FLOAT_H
#define DIP_NODE_FLOAT_H

#include <climits>
#include <optional>
#include <snt/dip/nodes/node_value.h>

namespace snt::dip {

    class FloatNode : public ValueNode {
        val::BaseValue::PointerType cast_scalar_value(const std::string& value_input) const override;
        val::BaseValue::PointerType cast_array_value(
            const val::Array::StringType& value_inputs, const val::Array::ShapeType& shape
        ) const override;

      public:
        static constexpr size_t max_float_size = sizeof(long double) * CHAR_BIT;
        static ValueNode::PointerType is_node(Parser& parser);
        FloatNode(const std::string& nm, const core::DataType vdt) : BaseNode(NodeDtype::Float), ValueNode(nm, vdt) {};
        FloatNode(const std::string& nm, val::BaseValue::PointerType val)
            : BaseNode(NodeDtype::Float), ValueNode(nm, std::move(val)) {};
        FloatNode(const std::string& nm, val::BaseValue::PointerType val, std::optional<puq::Quantity> quant)
            : BaseNode(NodeDtype::Float), ValueNode(nm, std::move(val), std::move(quant)) {};
        FloatNode(Parser& parser);
        BaseNode::ListType parse(Environment& env) override;
        ValueNode::PointerType clone(const std::string& nm) const override;
        std::string to_string(const core::StringFormatType& format = core::StringFormatType()) const override;
    };

} // namespace snt::dip

#endif // DIP_NODE_FLOAT_H
