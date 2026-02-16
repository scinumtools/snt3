#ifndef DIP_NODE_BOOLEAN_H
#define DIP_NODE_BOOLEAN_H

#include <snt/dip/nodes/node_base.h>
#include <snt/dip/nodes/node_value.h>

namespace snt::dip {

  class BooleanNode : public ValueNode {
    val::BaseValue::PointerType cast_scalar_value(const std::string& value_input) const override;
    val::BaseValue::PointerType cast_array_value(const val::Array::StringType& value_inputs,
                                                 const val::Array::ShapeType& shape) const override;

  public:
    static ValueNode::PointerType is_node(Parser& parser);
    BooleanNode(const std::string& nm, val::BaseValue::PointerType val)
        : BaseNode(NodeDtype::Boolean), ValueNode(nm, std::move(val)) {};
    BooleanNode(Parser& parser)
        : BaseNode(parser, NodeDtype::Boolean), ValueNode(val::DataType::Boolean) {};
    BaseNode::ListType parse(Environment& env) override;
    ValueNode::PointerType clone(const std::string& nm) const override;
    void validate_options() const override;
    std::string to_string(const snt::StringFormatType& format = snt::StringFormatType()) const override;
  };

} // namespace snt::dip

#endif // DIP_NODE_BOOLEAN_H
