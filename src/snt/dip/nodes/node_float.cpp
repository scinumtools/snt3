#include "../parsers.h"

#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_float.h>
#include <snt/dip/solvers/numerical_solver.h>

namespace snt::dip {

    ValueNode::PointerType FloatNode::is_node(Parser& parser) {
        if (parser.dtype_raw[1] == "float") {
            parser.part_dimension();
            if (parser.part_equal(false))
                parser.part_value();
            parser.part_units();
            parser.part_comment();
            return std::make_shared<FloatNode>(parser);
        }
        return nullptr;
    }

    FloatNode::FloatNode(const FloatNode& other) : ValueNode(other) {
        if (!value)
            value_dtype = core::DataType::Float64;
    }

    FloatNode::FloatNode(Parser& parser) : ValueNode(parser, NodeDtype::Float) {
        if (dtype_raw[2] == "32") {
            value_dtype = core::DataType::Float32;
        } else if (dtype_raw[2] == "64" || dtype_raw[2] == "") {
            value_dtype = core::DataType::Float64;
        } else if (dtype_raw[2] == "128" && max_float_size == 128) {
            value_dtype = core::DataType::Float128;
        } else {
            throw dip::SyntaxException(
                "Invalid float data type",
                "The float data type cannot be determined from the node settings.",
                "Use `32`, `64`, or `128` as the float size; `128` is only available when the maximum supported float "
                "size is 128.",
                __FILE__,
                __LINE__,
                line
            );
        }
    };

    BaseNode::ListType FloatNode::parse(Environment& env) {
        switch (value_origin) {
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
            break;
        }
        return {};
    }

    val::BaseValue::PointerType FloatNode::cast_scalar_value(const std::string& value_input) const {
        // TODO: variable precision x should be implemented
        switch (value_dtype) {
        case core::DataType::Float32:
            return std::make_unique<val::ArrayValueFloat32>(std::stof(value_input));
        case core::DataType::Float64:
            return std::make_unique<val::ArrayValueFloat64>(std::stod(value_input));
        case core::DataType::Float128:
            return std::make_unique<val::ArrayValueFloat128>(std::stold(value_input));
        default:
            throw dip::SyntaxException(
                "Invalid float cast",
                "The value cannot be cast to a `" + dtype_raw[2] + "`-bit float from the given string: `" +
                    value_input + "`.",
                "Provide a valid floating-point literal, such as `1.23`, `-4.5`, `1e-6`, or `inf`, that is "
                "representable by the selected float type.",
                __FILE__,
                __LINE__,
                line
            );
        }
    }

    val::BaseValue::PointerType FloatNode::cast_array_value(
        const val::Array::StringType& value_inputs, const val::Array::ShapeType& shape
    ) const {
        // TODO: variable precision x should be implemented
        switch (value_dtype) {
        case core::DataType::Float32: {
            std::vector<float> arr;
            if (std::any_of(shape.begin(), shape.end(), [](auto x) { return x != 0; })) {
                arr.reserve(value_inputs.size());
                for (const auto& s : value_inputs)
                    arr.push_back(std::stof(s));
            }
            return std::make_unique<val::ArrayValueFloat32>(arr, shape);
        }
        case core::DataType::Float64: {
            std::vector<double> arr;
            if (std::any_of(shape.begin(), shape.end(), [](auto x) { return x != 0; })) {
                arr.reserve(value_inputs.size());
                for (const auto& s : value_inputs)
                    arr.push_back(std::stod(s));
            }
            return std::make_unique<val::ArrayValueFloat64>(arr, shape);
        }
        case core::DataType::Float128: {
            std::vector<long double> arr;
            if (std::any_of(shape.begin(), shape.end(), [](auto x) { return x != 0; })) {
                arr.reserve(value_inputs.size());
                for (const auto& s : value_inputs)
                    arr.push_back(std::stold(s));
            }
            return std::make_unique<val::ArrayValueFloat128>(arr, shape);
        }
        default:
            std::ostringstream oss;
            for (const auto& s : value_inputs)
                oss << s;
            throw dip::SyntaxException(
                "Invalid float data type",
                "The array values cannot be cast to the specified `" + dtype_raw[2] + "`-bit float data type: `" +
                    oss.str() + "`.",
                "Use a supported floating-point data type: `32`, `64`, or `128`, and ensure that each array element is "
                "a valid floating-point literal representable by the selected type.",
                __FILE__,
                __LINE__,
                line
            );
        }
    }

    BaseNode::PointerType FloatNode::clone(const Path& pth, std::optional<size_t> indent) const {
        std::shared_ptr<FloatNode> copy = std::make_shared<FloatNode>(*this);
        copy->path = pth;
        if (indent)
            copy->indent = indent.value();
        return copy;
    }

    std::string FloatNode::to_string(const core::StringFormatType& format) const {
        std::stringstream ss;
        if (value) {
            ss << value->to_string(format);
            if (units)
                ss << " " << units->to_string();
        } else {
            return std::string(KEYWORD_NONE);
        }
        return ss.str();
    }

} // namespace snt::dip
