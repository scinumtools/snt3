#include "../parsers.h"

#include <snt/dip/environment.h>
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

    FloatNode::FloatNode(Parser& parser) : BaseNode(parser, NodeDtype::Float) {
        if (dtype_raw[2] == "32") {
            value_dtype = core::DataType::Float32;
        } else if (dtype_raw[2] == "64" or dtype_raw[2] == "") {
            value_dtype = core::DataType::Float64;
        } else if (dtype_raw[2] == "128" and max_float_size == 128) {
            value_dtype = core::DataType::Float128;
        } else {
            throw std::runtime_error("Value data type cannot be determined from the node settings");
        }
    };

    BaseNode::ListType FloatNode::parse(Environment& env) {
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
            ValueNodeData data = solver.eval(value_raw.at(0), units_raw);
            set_value(std::move(data.value));
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
            throw std::runtime_error(
                "Value cannot be casted as " + dtype_raw[2] + " bit float type from the given string: " + value_input
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
            arr.reserve(value_inputs.size());
            for (const auto& s : value_inputs)
                arr.push_back(std::stof(s));
            return std::make_unique<val::ArrayValueFloat32>(arr, shape);
        }
        case core::DataType::Float64: {
            std::vector<double> arr;
            arr.reserve(value_inputs.size());
            for (const auto& s : value_inputs)
                arr.push_back(std::stod(s));
            return std::make_unique<val::ArrayValueFloat64>(arr, shape);
        }
        case core::DataType::Float128: {
            std::vector<long double> arr;
            arr.reserve(value_inputs.size());
            for (const auto& s : value_inputs)
                arr.push_back(std::stold(s));
            return std::make_unique<val::ArrayValueFloat128>(arr, shape);
        }
        default:
            std::ostringstream oss;
            for (const auto& s : value_inputs)
                oss << s;
            throw std::runtime_error(
                "Value cannot be casted as " + dtype_raw[2] + " bit float type from the given string: " + oss.str()
            );
        }
    }

    ValueNode::PointerType FloatNode::clone(const Path& pth) const {
        if (value == nullptr)
            return std::make_shared<FloatNode>(pth, value->get_dtype());
        else if (!units)
            return std::make_shared<FloatNode>(pth, std::move(value->clone()));
        else
            return std::make_shared<FloatNode>(pth, std::move(value->clone()), units);
    }

    std::string FloatNode::to_string(const core::StringFormatType& format) const {
        std::stringstream ss;
        ss << value->to_string(format);
        if (units)
            ss << " " << units->to_string();
        return ss.str();
    }

} // namespace snt::dip
