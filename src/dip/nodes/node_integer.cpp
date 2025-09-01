#include "../parsers.h"
#include "../solvers/solvers.h"
#include "nodes.h"

namespace snt::dip {

  BaseNode::PointerType IntegerNode::is_node(Parser& parser) {
    if (parser.dtype_raw[1] == "int") {
      parser.part_dimension();
      if (parser.part_equal(false))
        parser.part_value();
      parser.part_units();
      parser.part_comment();
      return std::make_shared<IntegerNode>(parser);
    }
    return nullptr;
  }

  IntegerNode::IntegerNode(Parser& parser) : BaseNode(parser, NodeDtype::Integer) {
    if (dtype_raw[2] == "16") {
      value_dtype = (dtype_raw[0] == "u") ? val::DataType::Integer16_U : val::DataType::Integer16;
    } else if (dtype_raw[2] == "32" or dtype_raw[2] == "") {
      value_dtype = (dtype_raw[0] == "u") ? val::DataType::Integer32_U : val::DataType::Integer32;
    } else if (dtype_raw[2] == "64") {
      value_dtype = (dtype_raw[0] == "u") ? val::DataType::Integer64_U : val::DataType::Integer64;
    } else {
      throw std::runtime_error("Value data type cannot be determined from the node settings");
    }
  };

  BaseNode::NodeListType IntegerNode::parse(Environment& env) {
    switch (value_origin) {
    case ValueOrigin::Function:
      set_value(env.request_value(value_raw.at(0), RequestType::Function, units_raw));
      break;
    case ValueOrigin::Reference: {
      set_value(env.request_value(value_raw.at(0), RequestType::Reference, units_raw));
      break;
    }
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

  val::BaseValue::PointerType IntegerNode::cast_scalar_value(const std::string& value_input) const {
    // TODO: variable precision x should be implemented
    switch (value_dtype) {
    case val::DataType::Integer16_U:
      return std::make_unique<val::ArrayValue<uint16_t>>((unsigned short)std::stoi(value_input));
      break;
    case val::DataType::Integer16:
      return std::make_unique<val::ArrayValue<int16_t>>((short)std::stoi(value_input));
      break;
    case val::DataType::Integer32_U:
      return std::make_unique<val::ArrayValue<uint32_t>>(std::stoi(value_input));
      break;
    case val::DataType::Integer32:
      return std::make_unique<val::ArrayValue<int32_t>>(std::stoi(value_input));
      break;
    case val::DataType::Integer64_U:
      return std::make_unique<val::ArrayValue<uint64_t>>(std::stoull(value_input));
      break;
    case val::DataType::Integer64:
      return std::make_unique<val::ArrayValue<int64_t>>(std::stoll(value_input));
      break;
    default:
      if (dtype_raw[0] == "u")
        throw std::runtime_error("Value cannot be casted as unsigned " + dtype_raw[0] +
                                 " bit integer type from the given string: " + value_input);
      else
        throw std::runtime_error("Value cannot be casted as " + dtype_raw[0] +
                                 " bit integer type from the given string: " + value_input);
    }
  }

  val::BaseValue::PointerType IntegerNode::cast_array_value(const val::Array::StringType& value_inputs,
                                                            const val::Array::ShapeType& shape) const {
    // TODO: variable precision x should be implemented
    switch (value_dtype) {
    case val::DataType::Integer16_U: {
      std::vector<uint16_t> arr;
      arr.reserve(value_inputs.size());
      for (const auto& s : value_inputs)
        arr.push_back((unsigned short)std::stoul(s));
      return std::make_unique<val::ArrayValue<uint16_t>>(arr, shape);
    }
    case val::DataType::Integer16: {
      std::vector<int16_t> arr;
      arr.reserve(value_inputs.size());
      for (const auto& s : value_inputs)
        arr.push_back((short)std::stoi(s));
      return std::make_unique<val::ArrayValue<int16_t>>(arr, shape);
    }
    case val::DataType::Integer32_U: {
      std::vector<uint32_t> arr;
      arr.reserve(value_inputs.size());
      for (const auto& s : value_inputs)
        arr.push_back(std::stoul(s));
      return std::make_unique<val::ArrayValue<uint32_t>>(arr, shape);
    }
    case val::DataType::Integer32: {
      std::vector<int32_t> arr;
      arr.reserve(value_inputs.size());
      for (const auto& s : value_inputs)
        arr.push_back(std::stoi(s));
      return std::make_unique<val::ArrayValue<int32_t>>(arr, shape);
    }
    case val::DataType::Integer64_U: {
      std::vector<uint64_t> arr;
      arr.reserve(value_inputs.size());
      for (const auto& s : value_inputs)
        arr.push_back(std::stoull(s));
      return std::make_unique<val::ArrayValue<uint64_t>>(arr, shape);
    }
    case val::DataType::Integer64: {
      std::vector<int64_t> arr;
      arr.reserve(value_inputs.size());
      for (const auto& s : value_inputs)
        arr.push_back(std::stoll(s));
      return std::make_unique<val::ArrayValue<int64_t>>(arr, shape);
    }
    default:
      std::ostringstream oss;
      for (const auto& s : value_inputs)
        oss << s;
      if (dtype_raw[0] == "u") {
        throw std::runtime_error("Value cannot be casted as unsigned " + dtype_raw[0] +
                                 " bit integer type from the given string: " + oss.str());
      } else {
        throw std::runtime_error("Value cannot be casted as " + dtype_raw[0] +
                                 " bit integer type from the given string: " + oss.str());
      }
    }
  }

  BaseNode::PointerType IntegerNode::clone(const std::string& nm) const {
    if (value == nullptr)
      return std::make_shared<IntegerNode>(nm, value->get_dtype());
    else
      return std::make_shared<IntegerNode>(nm, std::move(value->clone()));
  }

} // namespace snt::dip
