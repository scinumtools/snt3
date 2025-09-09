#ifndef DIP_NODE_REAL_H
#define DIP_NODE_REAL_H

#include "node_quantity.h"

namespace snt::dip {

  class RealNode : public QuantityNode {
    val::BaseValue::PointerType cast_scalar_value(const std::string& value_input) const override;
    val::BaseValue::PointerType cast_array_value(const val::Array::StringType& value_inputs,
                                                 const val::Array::ShapeType& shape) const override;

  public:
    static constexpr size_t max_real_size = sizeof(long double) * CHAR_BIT;
    static BaseNode::PointerType is_node(Parser& parser);
    RealNode(const std::string& nm, const val::DataType vdt)
        : BaseNode(NodeDtype::Real), ValueNode(nm, vdt) {};
    RealNode(const std::string& nm, val::BaseValue::PointerType val)
        : BaseNode(NodeDtype::Real), ValueNode(nm, std::move(val)) {};
    RealNode(Parser& parser);
    BaseNode::NodeListType parse(Environment& env) override;
    BaseNode::PointerType clone(const std::string& nm) const override;
  };

} // namespace snt::dip

#endif // DIP_NODE_REAL_H
