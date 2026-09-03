#include "../parsers.h"

#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
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

    IntegerNode::IntegerNode(const IntegerNode& other) : ValueNode(other) {
        if (!value)
            value_dtype = core::DataType::Integer64;
    }

    IntegerNode::IntegerNode(Parser& parser) : ValueNode(parser, NodeDtype::Integer) {
        if (dtype_raw[2] == "16") {
            value_dtype = (dtype_raw[0] == "u") ? core::DataType::Integer16_U : core::DataType::Integer16;
        } else if (dtype_raw[2] == "32" || dtype_raw[2] == "") {
            value_dtype = (dtype_raw[0] == "u") ? core::DataType::Integer32_U : core::DataType::Integer32;
        } else if (dtype_raw[2] == "64") {
            value_dtype = (dtype_raw[0] == "u") ? core::DataType::Integer64_U : core::DataType::Integer64;
        } else {
            throw dip::SyntaxException(
                "Invalid integer data type",
                "The integer data type cannot be determined from the node settings.",
                "Use `16`, `32`, or `64` as the integer size, optionally prefixed with `u` for an unsigned integer.",
                __FILE__,
                __LINE__,
                line
            );
        }
    };

    BaseNode::ListType IntegerNode::parse(Environment& env) {
        switch (value_origin) {
        case ValueOrigin::FunctionRes:
            break;
        case ValueOrigin::Function:
            set_value(parse_function(env, value_raw.at(0), units_raw));
            break;
        case ValueOrigin::Reference:
        case ValueOrigin::ReferenceRel:
        case ValueOrigin::ReferenceRaw:
            set_value(parse_reference(env, value_raw.at(0), units_raw, value_origin));
            break;
        case ValueOrigin::Expression: {
            set_value(parse_expression(env, value_raw.at(0), units_raw, dtype));
            break;
        }
        default:
            set_value();
            break;
        }
        set_units();
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
        default: {
            const bool is_unsigned = dtype_raw[0] == "u";
            const std::string integer_type = is_unsigned ? "unsigned integer" : "signed integer";
            throw dip::SyntaxException(
                "Invalid integer cast",
                "The scalar value cannot be cast to the specified " + integer_type + " type from the given string: `" +
                    value_input + "`.",
                is_unsigned ? "Use a valid non-negative integer within the range of the selected unsigned integer type."
                            : "Use a valid integer within the range of the selected signed integer type.",
                __FILE__,
                __LINE__,
                line
            );
        }
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
        default: {
            std::ostringstream oss;
            for (const auto& s : value_inputs)
                oss << s;

            const bool is_unsigned = dtype_raw[0] == "u";
            const std::string integer_type = is_unsigned ? "unsigned integer" : "signed integer";

            throw dip::SyntaxException(
                "Invalid integer cast",
                "The array values cannot be cast to the specified " + integer_type + " type from the given strings: `" +
                    oss.str() + "`.",
                is_unsigned ? "Use valid non-negative integer values within the range of the selected unsigned "
                              "integer type."
                            : "Use valid integer values within the range of the selected signed integer type.",
                __FILE__,
                __LINE__,
                line
            );
        }
        }
    }

    BaseNode::PointerType IntegerNode::clone(const Path& pth, std::optional<size_t> indent) const {
        std::shared_ptr<IntegerNode> copy = std::make_shared<IntegerNode>(*this);
        copy->path = pth;
        if (indent)
            copy->indent = indent.value();
        return copy;
    }

    std::string IntegerNode::to_string(const core::StringFormatType& format) const {
        std::stringstream ss;
        if (value) {
            ss << value->to_string(format);
            if (units)
                ss << " " << units->to_string();
        } else {
            ss << std::string(KEYWORD_NONE);
        }
        return ss.str();
    }

} // namespace snt::dip
