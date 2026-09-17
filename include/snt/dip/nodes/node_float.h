#ifndef DIP_NODE_FLOAT_H
#define DIP_NODE_FLOAT_H

#include <climits>
#include <optional>
#include <snt/dip/nodes/node_value.h>

namespace snt::dip {

    /** DIPL value node containing floating-point data. */
    class FloatNode : public ValueNode {
        val::BaseValue::PointerType cast_scalar_value(const std::string& value_input) const override;
        val::BaseValue::PointerType cast_array_value(
            const val::Array::StringType& value_inputs, const val::Array::ShapeType& shape
        ) const override;

      public:
        static constexpr size_t max_float_size = sizeof(long double) * CHAR_BIT;
        /** Return a floating-point node when the parser is positioned at a numeric literal.
         * @param parser Parser state to inspect.
         */
        static ValueNode::PointerType is_node(Parser& parser);
        /** Copy a floating-point node. @param other Node to copy. */
        FloatNode(const FloatNode& other);
        /** Construct a floating-point node with a path and data type flag. */
        FloatNode(const Path& pth, const core::DataType vdt) : ValueNode(pth, vdt, NodeDtype::Float) {};
        /** Construct a floating-point node from a value at a path. */
        FloatNode(const Path& pth, val::BaseValue::PointerType val)
            : ValueNode(pth, std::move(val), NodeDtype::Float) {};
        /** Construct a floating-point node with an optional quantity. */
        FloatNode(const Path& pth, val::BaseValue::PointerType val, std::optional<puq::Quantity> quant)
            : ValueNode(pth, std::move(val), NodeDtype::Float, std::move(quant)) {};
        /** Construct a floating-point node from parser input. */
        FloatNode(Parser& parser);
        BaseNode::ListType parse(Environment& env) override;
        BaseNode::PointerType clone(const Path& pth, std::optional<size_t> indent = std::nullopt) const override;
        std::string to_string(const core::StringFormatType& format = core::StringFormatType()) const override;
    };

} // namespace snt::dip

#endif // DIP_NODE_FLOAT_H
