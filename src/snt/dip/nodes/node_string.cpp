#include "node_string.h"

#include "../parsers.h"

#include <regex>

namespace snt::dip {

  BaseNode::PointerType StringNode::is_node(Parser& parser) {
    if (parser.dtype_raw[1] == "str") {
      parser.part_dimension();
      if (parser.part_equal(false))
        parser.part_value();
      parser.part_units();
      parser.part_comment();
      return std::make_shared<StringNode>(parser);
    }
    return nullptr;
  }

  BaseNode::NodeListType StringNode::parse(Environment& env) {
    if (!units_raw.empty())
      throw std::runtime_error("String data type does not support units: " + line.code);
    switch (value_origin) {
    case ValueOrigin::Function:
      set_value(env.request_value(value_raw.at(0), RequestType::Function));
      break;
    case ValueOrigin::Reference:
      set_value(env.request_value(value_raw.at(0), RequestType::Reference));
      break;
    case ValueOrigin::ReferenceRaw: {
      std::string source_code = env.request_code(value_raw.at(0));
      val::Array::StringType source_value_raw;
      val::Array::ShapeType source_value_shape;
      parse_value(source_code, source_value_raw, source_value_shape);
      set_value(cast_value(source_value_raw, source_value_shape));
      break;
    }
    case ValueOrigin::Expression: {
      // TODO: implement template expression
      break;
    }
    default:
      break;
    }
    // TODO: process expression
    return {};
  }

  val::BaseValue::PointerType StringNode::cast_scalar_value(const std::string& value_input) const {
    return std::make_unique<val::ArrayValue<std::string>>(value_input);
  }

  val::BaseValue::PointerType StringNode::cast_array_value(const val::Array::StringType& value_inputs,
                                                           const val::Array::ShapeType& shape) const {
    return std::make_unique<val::ArrayValue<std::string>>(value_inputs, shape);
  }

  ValueNode::PointerType StringNode::clone(const std::string& nm) const {
    if (value == nullptr)
      return std::make_shared<StringNode>(nm, nullptr);
    else
      return std::make_shared<StringNode>(nm, std::move(value->clone()));
  }

  bool StringNode::set_property(PropertyType property, val::Array::StringType& values,
                                std::string& units) {
    if (ValueNode::set_property(property, values, units)) {
      return true;
    } else if (property == PropertyType::Format) {
      this->format = values.at(0);
      return true;
    } else {
      return false;
    }
  }

  void StringNode::validate_format() const {
    if (format.size() > 0) {
      std::regex pattern(format);
      const val::ArrayValue<std::string> valueT(value.get());
      for (int i = 0; i < valueT.get_size(); i++) {
        if (!std::regex_match(valueT.get_value(i), pattern)) {
          throw std::runtime_error("Node value " + value->to_string() +
                                   " does not match with expected format '" + format + "'");
        }
      }
    }
  }

  std::string StringNode::to_string(const snt::StringFormatType& format) const {
    return value->to_string();
  }
  
} // namespace snt::dip
