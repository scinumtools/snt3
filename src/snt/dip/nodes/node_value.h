#ifndef DIP_NODE_VALUE_H
#define DIP_NODE_VALUE_H

namespace snt::dip {

  class ValueNode : virtual public BaseNode {
    virtual val::BaseValue::PointerType cast_scalar_value(const std::string& value_input) const = 0;
    virtual val::BaseValue::PointerType cast_array_value(const val::Array::StringType& value_inputs,
                                                         const val::Array::ShapeType& shape) const = 0;

  protected:
    struct OptionStruct {
      val::BaseValue::PointerType value;
      std::string value_raw;
      std::string units_raw;
    };
    val::DataType value_dtype;

  public:
    typedef std::shared_ptr<ValueNode> PointerType;
    val::BaseValue::PointerType value;
    val::Array::StringType tags;
    bool constant;
    std::string description;
    std::string condition;
    std::vector<OptionStruct> options;
    std::string format;
    ValueNode() : constant(false) {};
    ValueNode(const val::DataType vdt) : constant(false), value_dtype(vdt) {};
    ValueNode(const std::string& nm, const val::DataType vdt);
    ValueNode(const std::string& nm, val::BaseValue::PointerType val);
    virtual ~ValueNode() = default;
    val::BaseValue::PointerType cast_value();
    val::BaseValue::PointerType cast_value(val::Array::StringType& value_input,
                                           const val::Array::ShapeType& shape);
    void set_value(val::BaseValue::PointerType value_input = nullptr);
    void modify_value(const BaseNode::PointerType& node, Environment& env);
    virtual ValueNode::PointerType clone(const std::string& nm) const = 0;
    virtual bool set_property(PropertyType property, val::Array::StringType& values,
                              std::string& units) override;
    virtual std::string to_string() const = 0;
    void validate_constant() const;
    void validate_definition() const;
    void validate_condition() const;
    virtual void validate_options() const;
    virtual void validate_format() const;

  private:
    void validate_dimensions() const;
  };

} // namespace snt::dip

#endif // DIP_NODE_VALUE_H
