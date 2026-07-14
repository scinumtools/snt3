#include "../parsers.h"

#include <snt/dip/environment.h>
#include <snt/dip/nodes/node_integer.h>
#include <snt/dip/solvers/numerical_solver.h>

namespace snt::dip {

    ValueNode::PointerType IntegerNode::is_node(Parser& parser) {
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
            value_dtype = (dtype_raw[0] == "u") ? core::DataType::Integer16_U : core::DataType::Integer16;
        } else if (dtype_raw[2] == "32" || dtype_raw[2] == "") {
            value_dtype = (dtype_raw[0] == "u") ? core::DataType::Integer32_U : core::DataType::Integer32;
        } else if (dtype_raw[2] == "64") {
            value_dtype = (dtype_raw[0] == "u") ? core::DataType::Integer64_U : core::DataType::Integer64;
        } else {
            throw std::runtime_error("Value data type cannot be determined from the node settings");
        }
    };

    BaseNode::ListType IntegerNode::parse(Environment& env) {
        switch (value_origin) {
        case ValueOrigin::Function:
            set_value(env.request_value(value_raw.at(0), RequestType::Function, units_raw));
            break;
        case ValueOrigin::Reference: {
            val::BaseValue::PointerType val = env.request_value(value_raw.at(0), RequestType::Reference, units_raw);
            if (val)
                set_value(std::move(val));
            else
                throw std::runtime_error("Value environment request returns an empty pointer: " + value_raw.at(0));
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
            ValueNodeData data = solver.eval(value_raw.at(0), units_raw);
            set_value(std::move(data.value));
            break;
        }
        default:
            break;
        }
        return {};
    }

    val::BaseValue::PointerType IntegerNode::cast_scalar_value(const std::string& value_input) const {
        // TODO: variable precision x should be implemented
        switch (value_dtype) {
        case core::DataType::Integer16_U:
            return std::make_unique<val::ArrayValueUint16>((unsigned short)std::stoi(value_input));
            break;
        case core::DataType::Integer16:
            return std::make_unique<val::ArrayValueInt16>((short)std::stoi(value_input));
            break;
        case core::DataType::Integer32_U:
            return std::make_unique<val::ArrayValueUint32>(std::stoi(value_input));
            break;
        case core::DataType::Integer32:
            return std::make_unique<val::ArrayValueInt32>(std::stoi(value_input));
            break;
        case core::DataType::Integer64_U:
            return std::make_unique<val::ArrayValueUint64>(std::stoull(value_input));
            break;
        case core::DataType::Integer64:
            return std::make_unique<val::ArrayValueInt64>(std::stoll(value_input));
            break;
        default:
            if (dtype_raw[0] == "u")
                throw std::runtime_error(
                    "Value cannot be casted as unsigned " + dtype_raw[0] +
                    " bit integer type from the given string: " + value_input
                );
            else
                throw std::runtime_error(
                    "Value cannot be casted as " + dtype_raw[0] +
                    " bit integer type from the given string: " + value_input
                );
        }
    }

    val::BaseValue::PointerType IntegerNode::cast_array_value(
        const val::Array::StringType& value_inputs, const val::Array::ShapeType& shape
    ) const {
        // TODO: variable precision x should be implemented
        switch (value_dtype) {
        case core::DataType::Integer16_U: {
            std::vector<uint16_t> arr;
            if (std::any_of(shape.begin(), shape.end(), [](auto x) { return x != 0; })) {
                arr.reserve(value_inputs.size());
                for (const auto& s : value_inputs)
                    arr.push_back((unsigned short)std::stoul(s));
            }
            return std::make_unique<val::ArrayValueUint16>(arr, shape);
        }
        case core::DataType::Integer16: {
            std::vector<int16_t> arr;
            if (std::any_of(shape.begin(), shape.end(), [](auto x) { return x != 0; })) {
                arr.reserve(value_inputs.size());
                for (const auto& s : value_inputs)
                    arr.push_back((short)std::stoi(s));
            }
            return std::make_unique<val::ArrayValueInt16>(arr, shape);
        }
        case core::DataType::Integer32_U: {
            std::vector<uint32_t> arr;
            if (std::any_of(shape.begin(), shape.end(), [](auto x) { return x != 0; })) {
                arr.reserve(value_inputs.size());
                for (const auto& s : value_inputs)
                    arr.push_back(std::stoul(s));
            }
            return std::make_unique<val::ArrayValueUint32>(arr, shape);
        }
        case core::DataType::Integer32: {
            std::vector<int32_t> arr;
            if (std::any_of(shape.begin(), shape.end(), [](auto x) { return x != 0; })) {
                arr.reserve(value_inputs.size());
                for (const auto& s : value_inputs)
                    arr.push_back(std::stoi(s));
            }
            return std::make_unique<val::ArrayValueInt32>(arr, shape);
        }
        case core::DataType::Integer64_U: {
            std::vector<uint64_t> arr;
            if (std::any_of(shape.begin(), shape.end(), [](auto x) { return x != 0; })) {
                arr.reserve(value_inputs.size());
                for (const auto& s : value_inputs)
                    arr.push_back(std::stoull(s));
            }
            return std::make_unique<val::ArrayValueUint64>(arr, shape);
        }
        case core::DataType::Integer64: {
            std::vector<int64_t> arr;
            if (std::any_of(shape.begin(), shape.end(), [](auto x) { return x != 0; })) {
                arr.reserve(value_inputs.size());
                for (const auto& s : value_inputs)
                    arr.push_back(std::stoll(s));
            }
            return std::make_unique<val::ArrayValueInt64>(arr, shape);
        }
        default:
            std::ostringstream oss;
            for (const auto& s : value_inputs)
                oss << s;
            if (dtype_raw[0] == "u") {
                throw std::runtime_error(
                    "Value cannot be casted as unsigned " + dtype_raw[0] +
                    " bit integer type from the given string: " + oss.str()
                );
            } else {
                throw std::runtime_error(
                    "Value cannot be casted as " + dtype_raw[0] +
                    " bit integer type from the given string: " + oss.str()
                );
            }
        }
    }

    ValueNode::PointerType IntegerNode::clone(const Path& pth) const {
        if (value == nullptr)
            return std::make_shared<IntegerNode>(pth, nullptr);
        else if (!units)
            return std::make_shared<IntegerNode>(pth, std::move(value->clone()));
        else
            return std::make_shared<IntegerNode>(pth, std::move(value->clone()), units);
    }

    std::string IntegerNode::to_string(const core::StringFormatType& format) const {
        std::stringstream ss;
        ss << value->to_string(format);
        if (units)
            ss << " " << units->to_string();
        return ss.str();
    }

} // namespace snt::dip
