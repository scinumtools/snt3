#ifndef DIP_NODE_STRING_H
#define DIP_NODE_STRING_H

#include "node_value.h"

namespace snt::dip {

  class StringNode : public ValueNode {
    val::BaseValue::PointerType cast_scalar_value(const std::string& value_input) const override;
    val::BaseValue::PointerType cast_array_value(const val::Array::StringType& value_inputs,
                                                 const val::Array::ShapeType& shape) const override;

  public:
    static BaseNode::PointerType is_node(Parser& parser);
    StringNode(const std::string& nm, val::BaseValue::PointerType val)
        : BaseNode(NodeDtype::String), ValueNode(nm, std::move(val)) {};
    StringNode(Parser& parser)
        : BaseNode(parser, NodeDtype::String), ValueNode(val::DataType::String) {};
    BaseNode::NodeListType parse(Environment& env) override;
    ValueNode::PointerType clone(const std::string& nm) const override;
    bool set_property(PropertyType property, val::Array::StringType& values,
                      std::string& units) override;
    void validate_format() const override;
    std::string to_string() const override;
  };

} // namespace snt::dip

#endif // DIP_NODE_STRING_H
