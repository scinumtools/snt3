#ifndef DIP_NODE_INTEGER_H
#define DIP_NODE_INTEGER_H

#include <climits>
#include <optional>
#include <snt/dip/nodes/node_value.h>

namespace snt::dip {

    /** DIPL value node containing integer data. */
    class IntegerNode : public ValueNode {
        val::BaseValue::PointerType cast_scalar_value(const std::string& value_input) const override;
        val::BaseValue::PointerType cast_array_value(
            const val::Array::StringType& value_inputs, const val::Array::ShapeType& shape
        ) const override;

      public:
        static constexpr size_t max_int_size = sizeof(long long) * CHAR_BIT;
        /** Return an integer node when the parser is positioned at an integer literal.
         * @param parser Parser state to inspect.
         */
        static ValueNode::PointerType is_node(Parser& parser);
        /** Copy an integer node. @param other Node to copy. */
        IntegerNode(const IntegerNode& other);
        /** Construct an integer node with a path and data type flag. */
        IntegerNode(const Path& pth, const core::DataType vdt) : ValueNode(pth, vdt, NodeDtype::Integer) {};
        /** Construct an integer node from a value at a path. */
        IntegerNode(const Path& pth, val::BaseValue::PointerType val)
            : ValueNode(pth, std::move(val), NodeDtype::Integer) {};
        /** Construct an integer node with an optional quantity. */
        IntegerNode(const Path& pth, val::BaseValue::PointerType val, std::optional<puq::Quantity> quant)
            : ValueNode(pth, std::move(val), NodeDtype::Integer, std::move(quant)) {};
        /** Construct an integer node from parser input. */
        IntegerNode(Parser& parser);
        BaseNode::ListType parse(Environment& env) override;
        BaseNode::PointerType clone(const Path& pth, std::optional<size_t> indent = std::nullopt) const override;
        std::string to_string(const core::StringFormatType& format = core::StringFormatType()) const override;
    };

} // namespace snt::dip

#endif // DIP_NODE_INTEGER_H
