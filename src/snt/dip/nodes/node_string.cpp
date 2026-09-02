#include "../parsers.h"

#include <regex>
#include <snt/dip/environment.h>
#include <snt/dip/nodes/node_string.h>
#include <snt/dip/solvers/template_solver.h>

namespace snt::dip {

    ValueNode::PointerType StringNode::is_node(Parser& parser) {
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

    StringNode::StringNode(const StringNode& other) : ValueNode(other) {
        if (!value)
            value_dtype = core::DataType::String;
    }

    BaseNode::ListType StringNode::parse(Environment& env) {
        if (!units_raw.empty())
            throw dip::UnitException(
                "Invalid units",
                "The string data type does not support units.",
                "Remove the units from the string node.",
                __FILE__,
                __LINE__,
                line
            );
        switch (value_origin) {
        case ValueOrigin::Function:
            set_value(parse_function(env, value_raw.at(0), std::nullopt));
            break;
        case ValueOrigin::Reference:
        case ValueOrigin::ReferenceRel:
        case ValueOrigin::ReferenceRaw:
            set_value(parse_reference(env, value_raw.at(0), std::nullopt, value_origin));
            break;
        case ValueOrigin::Keyword:
        case ValueOrigin::Number:
        case ValueOrigin::Expression: {
            set_value(parse_expression(env, value_raw.at(0), std::nullopt, dtype));
            break;
        }
        default:
            break;
        }
        return {};
    }

    val::BaseValue::PointerType StringNode::cast_scalar_value(const std::string& value_input) const {
        return std::make_unique<val::ArrayValueStr>(value_input);
    }

    val::BaseValue::PointerType StringNode::cast_array_value(
        const val::Array::StringType& value_inputs, const val::Array::ShapeType& shape
    ) const {
        if (std::any_of(shape.begin(), shape.end(), [](auto x) { return x != 0; })) {
            return std::make_unique<val::ArrayValueStr>(value_inputs, shape);
        } else {
            return std::make_unique<val::ArrayValueStr>(std::vector<std::string>{}, shape);
        }
    }

    BaseNode::PointerType StringNode::clone(const Path& pth, std::optional<size_t> indent) const {
        std::shared_ptr<StringNode> copy = std::make_shared<StringNode>(*this);
        copy->path = pth;
        if (indent)
            copy->indent = indent.value();
        return copy;
    }

    bool StringNode::set_property(PropertyType property, val::Array::StringType& values, std::string& units) {
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
            const val::ArrayValueStr valueT(value.get());
            for (int i = 0; i < valueT.get_size(); i++) {
                if (!std::regex_match(valueT.get_value(i), pattern)) {
                    throw dip::SyntaxException(
                        "Format mismatch",
                        "The node value `" + valueT.get_value(i) + "` does not match the expected format `" + format +
                            "`.",
                        "Provide a string value that matches the specified regular expression format.",
                        __FILE__,
                        __LINE__,
                        line
                    );
                }
            }
        }
    }

    std::string StringNode::to_string(const core::StringFormatType& format) const {
        if (value)
            return value->to_string(format);
        else
            return std::string(KEYWORD_NONE);
    }

} // namespace snt::dip
