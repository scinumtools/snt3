#include "node_real.h"

#include "../parsers.h"
#include "../solvers/solver_numerical.h"

namespace snt::dip {

  BaseNode::PointerType RealNode::is_node(Parser& parser) {
    if (parser.dtype_raw[1] == "real") {
      parser.part_dimension();
      if (parser.part_equal(false))
        parser.part_value();
      parser.part_units();
      parser.part_comment();
      return std::make_shared<RealNode>(parser);
    }
    return nullptr;
  }

  RealNode::RealNode(Parser& parser) : BaseNode(parser, NodeDtype::Real) {
    if (dtype_raw[2] == "32") {
      value_dtype = val::DataType::Real32;
    } else if (dtype_raw[2] == "64" or dtype_raw[2] == "") {
      value_dtype = val::DataType::Real64;
    } else if (dtype_raw[2] == "128" and max_real_size == 128) {
      value_dtype = val::DataType::Real128;
    } else {
      throw std::runtime_error("Value data type cannot be determined from the node settings");
    }
  };

  BaseNode::NodeListType RealNode::parse(Environment& env) {
    switch (value_origin) {
    case ValueOrigin::Function:
      set_value(env.request_value(value_raw.at(0), RequestType::Function, units_raw));
      break;
    case ValueOrigin::Reference:
      set_value(env.request_value(value_raw.at(0), RequestType::Reference, units_raw));
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
      NumericalSolver solver(env);
      NumericalAtom result = solver.eval(value_raw.at(0));
      set_value(std::move(result.value));
      break;
    }
    default:
      break;
    }
    // TODO: process expression
    return {};
  }

  val::BaseValue::PointerType RealNode::cast_scalar_value(const std::string& value_input) const {
    // TODO: variable precision x should be implemented
    switch (value_dtype) {
    case val::DataType::Real32:
      return std::make_unique<val::ArrayValue<float>>(std::stof(value_input));
    case val::DataType::Real64:
      return std::make_unique<val::ArrayValue<double>>(std::stod(value_input));
    case val::DataType::Real128:
      return std::make_unique<val::ArrayValue<long double>>(std::stold(value_input));
    default:
      throw std::runtime_error("Value cannot be casted as " + dtype_raw[2] +
                               " bit real type from the given string: " + value_input);
    }
  }

  val::BaseValue::PointerType RealNode::cast_array_value(const val::Array::StringType& value_inputs,
                                                          const val::Array::ShapeType& shape) const {
    // TODO: variable precision x should be implemented
    switch (value_dtype) {
    case val::DataType::Real32: {
      std::vector<float> arr;
      arr.reserve(value_inputs.size());
      for (const auto& s : value_inputs)
        arr.push_back(std::stof(s));
      return std::make_unique<val::ArrayValue<float>>(arr, shape);
    }
    case val::DataType::Real64: {
      std::vector<double> arr;
      arr.reserve(value_inputs.size());
      for (const auto& s : value_inputs)
        arr.push_back(std::stod(s));
      return std::make_unique<val::ArrayValue<double>>(arr, shape);
    }
    case val::DataType::Real128: {
      std::vector<long double> arr;
      arr.reserve(value_inputs.size());
      for (const auto& s : value_inputs)
        arr.push_back(std::stold(s));
      return std::make_unique<val::ArrayValue<long double>>(arr, shape);
    }
    default:
      std::ostringstream oss;
      for (const auto& s : value_inputs)
        oss << s;
      throw std::runtime_error("Value cannot be casted as " + dtype_raw[2] +
                               " bit real type from the given string: " + oss.str());
    }
  }

  ValueNode::PointerType RealNode::clone(const std::string& nm) const {
    if (value == nullptr)
      return std::make_shared<RealNode>(nm, value->get_dtype());
    else if (units == nullptr)
      return std::make_shared<RealNode>(nm, std::move(value->clone()));
    else
      return std::make_shared<RealNode>(nm, std::move(value->clone()), std::move(units->clone()));
  }

} // namespace snt::dip
