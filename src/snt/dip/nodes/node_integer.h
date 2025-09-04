#ifndef DIP_NODE_INTEGER_H
#define DIP_NODE_INTEGER_H

#include "node_quantity.h"

namespace snt::dip {

  class IntegerNode : public QuantityNode {
    val::BaseValue::PointerType cast_scalar_value(const std::string& value_input) const override;
    val::BaseValue::PointerType cast_array_value(const val::Array::StringType& value_inputs,
                                                 const val::Array::ShapeType& shape) const override;

  public:
    static constexpr size_t max_int_size = sizeof(long long) * CHAR_BIT;
    static BaseNode::PointerType is_node(Parser& parser);
    IntegerNode(const std::string& nm, const val::DataType vdt)
        : BaseNode(NodeDtype::Integer), ValueNode(nm, vdt) {};
    IntegerNode(const std::string& nm, val::BaseValue::PointerType val)
        : BaseNode(NodeDtype::Integer), ValueNode(nm, std::move(val)) {};
    IntegerNode(Parser& parser);
    BaseNode::NodeListType parse(Environment& env) override;
    BaseNode::PointerType clone(const std::string& nm) const override;
  };

} // namespace snt::dip

#endif // DIP_NODE_INTEGER_H
