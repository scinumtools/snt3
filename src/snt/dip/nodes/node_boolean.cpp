#include "../parsers.h"

#include <algorithm>
#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_boolean.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/dip/solvers/logical_solver.h>
#include <stdexcept>

namespace snt::dip {

    ValueNode::PointerType BooleanNode::is_node(Parser& parser) {
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

    BooleanNode::BooleanNode(const BooleanNode& other) : ValueNode(other) {
        if (!value)
            value_dtype = core::DataType::Boolean;
    }

    BaseNode::ListType BooleanNode::parse(Environment& env) {
        if (!units_raw.empty())
            throw dip::UnitException(
                "Invalid units",
                "The boolean data type does not support units.",
                "Remove the units from the boolean node.",
                __FILE__,
                __LINE__,
                line
            );
        switch (value_origin) {
        case ValueOrigin::FunctionRes:
            break;
        case ValueOrigin::Function:
            set_value(parse_function(env, value_raw.at(0), std::nullopt));
            break;
        case ValueOrigin::Reference:
        case ValueOrigin::ReferenceRel:
        case ValueOrigin::ReferenceRaw:
            set_value(parse_reference(env, value_raw.at(0), std::nullopt, value_origin));
            break;
        case ValueOrigin::Expression: {
            set_value(parse_expression(env, value_raw.at(0), std::nullopt, dtype));
            break;
        }
        default:
            set_value();
            break;
        }
        return {};
    }

    val::BaseValue::PointerType BooleanNode::cast_scalar_value(const std::string& value_input) const {
        if (value_input == core::KEYWORD_TRUE)
            return std::make_unique<val::ArrayValueBool>(true);
        else if (value_input == core::KEYWORD_FALSE)
            return std::make_unique<val::ArrayValueBool>(false);
        else
            throw dip::SyntaxException(
                "Invalid boolean value",
                "The value cannot be cast to a boolean from the given string: `" + value_input + "`.",
                "Use `true` or `false` as the boolean value.",
                __FILE__,
                __LINE__,
                line
            );
    }

    val::BaseValue::PointerType BooleanNode::cast_array_value(
        const val::Array::StringType& value_inputs, const val::Array::ShapeType& shape
    ) const {
        std::vector<bool> bool_values;
        if (std::any_of(shape.begin(), shape.end(), [](auto x) { return x != 0; })) {
            bool_values.reserve(value_inputs.size());
            for (const auto& value : value_inputs) {
                if (value == core::KEYWORD_TRUE)
                    bool_values.push_back(true);
                else if (value == core::KEYWORD_FALSE)
                    bool_values.push_back(false);
                else
                    throw dip::SyntaxException(
                        "Invalid boolean value",
                        "The value cannot be cast to a boolean array from the given string: `" + value + "`.",
                        "Use `true` or `false` for every element of the boolean array.",
                        __FILE__,
                        __LINE__,
                        line
                    );
            }
        }
        return std::make_unique<val::ArrayValueBool>(bool_values, shape);
    }

    BaseNode::PointerType BooleanNode::clone(const Path& pth, std::optional<size_t> indent) const {
        std::shared_ptr<BooleanNode> copy = std::make_shared<BooleanNode>(*this);
        copy->path = pth;
        if (indent)
            copy->indent = indent.value();
        return copy;
    }

    void BooleanNode::validate_options() const {
        if (format.size() > 0)
            throw dip::SyntaxException(
                "Invalid property",
                "The `options` property is not supported for boolean nodes.",
                "Use `options` only with integer, float, or string nodes.",
                __FILE__,
                __LINE__,
                line
            );
    }

    std::string BooleanNode::to_string(const core::StringFormatType& format) const {
        if (value)
            return value->to_string();
        else
            return std::string(KEYWORD_NONE);
    }

} // namespace snt::dip
