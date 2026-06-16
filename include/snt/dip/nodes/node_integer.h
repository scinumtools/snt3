#ifndef DIP_NODE_INTEGER_H
#define DIP_NODE_INTEGER_H

#include <climits>
#include <optional>
#include <snt/dip/nodes/node_value.h>

namespace snt::dip {

    class IntegerNode : public ValueNode {
        val::BaseValue::PointerType cast_scalar_value(const std::string& value_input) const override;
        val::BaseValue::PointerType cast_array_value(
            const val::Array::StringType& value_inputs, const val::Array::ShapeType& shape
        ) const override;

      public:
        static constexpr size_t max_int_size = sizeof(long long) * CHAR_BIT;
        static ValueNode::PointerType is_node(Parser& parser);
        IntegerNode(const std::string& nm, const std::vector<CollectionAccess>& col, const core::DataType vdt)
            : BaseNode(NodeDtype::Integer), ValueNode(nm, col, vdt) {};
        IntegerNode(const std::string& nm, const std::vector<CollectionAccess>& col, val::BaseValue::PointerType val)
            : BaseNode(NodeDtype::Integer), ValueNode(nm, col, std::move(val)) {};
        IntegerNode(
            const std::string& nm,
            const std::vector<CollectionAccess>& col,
            val::BaseValue::PointerType val,
            std::optional<puq::Quantity> quant
        )
            : BaseNode(NodeDtype::Integer), ValueNode(nm, col, std::move(val), std::move(quant)) {};
        IntegerNode(Parser& parser);
        BaseNode::ListType parse(Environment& env) override;
        ValueNode::PointerType clone(const std::string& nm, const std::vector<CollectionAccess>& col) const override;
        std::string to_string(const core::StringFormatType& format = core::StringFormatType()) const override;
    };

} // namespace snt::dip

#endif // DIP_NODE_INTEGER_H
