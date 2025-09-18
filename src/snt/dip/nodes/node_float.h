#ifndef DIP_NODE_FLOAT_H
#define DIP_NODE_FLOAT_H

#include "node_quantity.h"

namespace snt::dip {

  class FloatNode : public QuantityNode {
    val::BaseValue::PointerType cast_scalar_value(const std::string& value_input) const override;
    val::BaseValue::PointerType cast_array_value(const val::Array::StringType& value_inputs,
                                                 const val::Array::ShapeType& shape) const override;

  public:
    static constexpr size_t max_float_size = sizeof(long double) * CHAR_BIT;
    static BaseNode::PointerType is_node(Parser& parser);
    FloatNode(const std::string& nm, const val::DataType vdt)
        : BaseNode(NodeDtype::Float), ValueNode(nm, vdt) {};
    FloatNode(const std::string& nm, val::BaseValue::PointerType val)
        : BaseNode(NodeDtype::Float), ValueNode(nm, std::move(val)) {};
    FloatNode(const std::string& nm, val::BaseValue::PointerType val, puq::Quantity::PointerType quant)
        : BaseNode(NodeDtype::Float), ValueNode(nm, std::move(val)), QuantityNode(std::move(quant)) {};
    FloatNode(Parser& parser);
    BaseNode::NodeListType parse(Environment& env) override;
    ValueNode::PointerType clone(const std::string& nm) const override;
  };

} // namespace snt::dip

#endif // DIP_NODE_FLOAT_H
