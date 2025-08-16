#include <stdexcept>

#include "../parsers.h"
#include "../solvers/solvers.h"
#include "nodes.h"

namespace dip {

  BaseNode::PointerType BooleanNode::is_node(Parser& parser) {
    if (parser.dtype_raw[1] == "bool") {
      parser.part_dimension();
      if (parser.part_equal(false))
        parser.part_value();
      parser.part_units();
      parser.part_comment();
      return std::make_shared<BooleanNode>(parser);
    }
    return nullptr;
  }

  BaseNode::NodeListType BooleanNode::parse(Environment& env) {
    if (!units_raw.empty())
      throw std::runtime_error("Boolean data type does not support units: " + line.code);
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
      LogicalSolver solver(env);
      LogicalAtom result = solver.eval(value_raw.at(0));
      set_value(std::move(result.value));
      break;
    }
    default:
      break;
    }
    // TODO: process expression
    return {};
  }

  val::BaseValue::PointerType BooleanNode::cast_scalar_value(const std::string& value_input) const {
    if (value_input == snt::KEYWORD_TRUE)
      return std::make_unique<val::ArrayValue<bool>>(true);
    else if (value_input == snt::KEYWORD_FALSE)
      return std::make_unique<val::ArrayValue<bool>>(false);
    else
      throw std::runtime_error("Value cannot be casted as boolean from the given string: " +
                               value_input);
  }

  val::BaseValue::PointerType BooleanNode::cast_array_value(const val::Array::StringType& value_inputs,
                                                            const val::Array::ShapeType& shape) const {
    std::vector<bool> bool_values;
    for (const auto& value : value_inputs) {
      if (value == snt::KEYWORD_TRUE)
        bool_values.push_back(true);
      else if (value == snt::KEYWORD_FALSE)
        bool_values.push_back(false);
      else
        throw std::runtime_error("Value cannot be casted as boolean from the given string: " +
                                 value);
    }
    return std::make_unique<val::ArrayValue<bool>>(bool_values, shape);
  }

  BaseNode::PointerType BooleanNode::clone(const std::string& nm) const {
    if (value == nullptr)
      return std::make_shared<BooleanNode>(nm, nullptr);
    else
      return std::make_shared<BooleanNode>(nm, std::move(value->clone()));
  }

  void BooleanNode::validate_options() const {
    if (format.size() > 0)
      throw std::runtime_error("Options property is not implemented for boolean nodes: " +
                               line.code);
  }

} // namespace dip
