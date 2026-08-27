#include <optional>
#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/dip/solvers/logical_solver.h>
#include <sstream>

namespace snt::dip {

    ValueNode::ValueNode(const ValueNode& other)
        : units(other.units), tags(other.tags), constant(other.constant), metadata(other.metadata),
          condition(other.condition), format(other.format), BaseNode(other) {
        options.reserve(other.options.size());
        for (const auto& option : other.options) {
            options.push_back({option.value->clone(), option.value_raw, option.units_raw});
        }
        if (other.value)
            value = other.value->clone();
        if (other.units)
            units = other.units;
    }

    ValueNode::ValueNode(const Path& pth, const core::DataType vdt, const NodeDtype dt)
        : constant(false), value_dtype(vdt) {
        dtype = dt;
        path = pth;
    }

    ValueNode::ValueNode(
        const Path& pth, val::BaseValue::PointerType val, const NodeDtype dt, std::optional<puq::Quantity> unt
    )
        : constant(false), value_dtype(val->get_dtype()), units(std::move(unt)), BaseNode(dt) {
        path = pth;
        val::Array::ShapeType dims = val->get_shape();
        if (val->get_size() > 1) {
            dimension.clear();
            dimension.reserve(dims.size());
            for (size_t dim : dims)
                dimension.push_back({dim, dim});
        }
        set_value(std::move(val));
    };

    ValueNode::ValueNode(const BaseNode::PointerType other, const NodeDtype dt, const core::DataType vdt)
        : value_dtype(vdt) {
        dtype = dt;
        path = other->path;
        indent = other->indent;
        line = other->line;
    }

    val::BaseValue::PointerType ValueNode::cast_value() {
        return cast_value(value_raw, value_shape);
    }

    val::BaseValue::PointerType ValueNode::cast_value(
        val::Array::StringType& value_input, const val::Array::ShapeType& shape
    ) {
        if (!dimension.empty()) {
            return cast_array_value(value_input, shape);
        } else if (value_input.empty()) {
            throw dip::SyntaxException(
                "Missing value",
                "The value node does not contain any value.",
                "Provide a value for the node or define it as an array if no scalar value is intended.",
                __FILE__,
                __LINE__,
                line
            );
        } else if (value_input.size() > 1) {
            throw dip::SyntaxException(
                "Unexpected array value",
                "The value node is defined as a scalar but contains multiple values.",
                "Provide exactly one value for a scalar node, or define the node with a dimension to accept an array.",
                __FILE__,
                __LINE__,
                line
            );
        } else {
            return cast_scalar_value(value_input.at(0));
        }
    }

    void ValueNode::set_value(val::BaseValue::PointerType value_input) {
        value = nullptr;
        if (value_input == nullptr && !value_raw.empty() && !value_raw.at(0).empty()) {
            value = cast_value();
        } else if (value_input == nullptr && value_origin == ValueOrigin::Array) {
            value = cast_value();
        } else if (value_input != nullptr) {
            if (value_input->get_dtype() == value_dtype)
                value = std::move(value_input);
            else
                value = value_input->cast_as(value_dtype);
        }
        if (value != nullptr) {
            if (!value_slice.empty()) {
                value = value->slice(value_slice);
            }
            if (dimension.empty()) {
                if (value->get_size() > 1)
                    throw dip::SyntaxException(
                        "Array assigned to scalar",
                        "The resulting value contains multiple elements but the node is defined as a scalar.",
                        "Assign a single-element value to the scalar node, or define the node with a dimension to "
                        "accept an array.",
                        __FILE__,
                        __LINE__,
                        line
                    );
                // throw std::runtime_error("Assigning array value to the scalar node: " + line.code);
            } else {
                validate_dimensions(); // check if value shape corresponds with dimension ranges
            }
        } else if (value_origin == ValueOrigin::None && !dimension.empty()) {
            validate_dimensions(); // check if value shape corresponds allows none
        }
    }

    void ValueNode::set_units(const std::optional<puq::Quantity>& units_input) {
        // setting node units
        units = std::nullopt;
        if (!units_input && !units_raw.empty()) {
            units = puq::Quantity(units_raw);
        } else if (units_input) {
            units = units_input;
        }
        // converting option units if necessary
        for (auto& option : options) {
            std::string option_units = option.units_raw;
            if (!option_units.empty()) {
                if (!units)
                    throw dip::SyntaxException(
                        "Dimension mismatch",
                        "The option specifies units `" + option_units + "` but the node is nondimensional.",
                        "Specify units for the node that are compatible with the option units.",
                        __FILE__,
                        __LINE__,
                        line
                    );
                else {
                    puq::Quantity quantity(std::move(option.value), option_units);
                    quantity = quantity.convert(*units);
                    option.value = std::move(quantity.measurement.result.estimate);
                }
            }
        }
    }

    void ValueNode::modify_value(const BaseNode::PointerType& node, Environment& env) {
        if (node->dtype != NodeDtype::Modification && node->dtype != dtype)
            throw dip::SyntaxException(
                "Type mismatch",
                "A node of type `" + dtype_raw.at(1) + "` cannot modify a node of type `" + node->dtype_raw.at(1) +
                    "`.",
                "Use a modification node or a node with a compatible type.",
                __FILE__,
                __LINE__,
                line
            );
        val::BaseValue::PointerType value = cast_value(node->value_raw, node->value_shape);
        if (!node->units_raw.empty()) {
            if (!this->units) {
                throw dip::UnitException(
                    "Dimension mismatch",
                    "The modification specifies units `" + node->units_raw + "` but the target node is nondimensional.",
                    "Specify units for the target node that are compatible with the modification units.",
                    __FILE__,
                    __LINE__,
                    line
                );
            } else {
                puq::Quantity quantity(std::move(value), node->units_raw);
                quantity = quantity.convert(*this->units);
                value = std::move(quantity.measurement.result.estimate);
            }
        }
        value_raw = node->value_raw;
        set_value(std::move(value));
    }

    bool ValueNode::set_property(PropertyType property, val::Array::StringType& values, std::string& units) {
        switch (property) {
            // directives
        case PropertyType::Options:
            for (const auto& value_option : values) {
                if (dtype == NodeDtype::Boolean)
                    throw dip::SyntaxException(
                        "Invalid property",
                        "The `Options` property is not supported for boolean nodes.",
                        "Use `Options` only with integer, float, or string nodes.",
                        __FILE__,
                        __LINE__,
                        line
                    );
                // TODO: account for multidimensional arrays as individual options
                val::BaseValue::PointerType ovalue = cast_scalar_value(value_option);
                options.push_back({std::move(ovalue), value_option, units});
            }
            return true;
        case PropertyType::Constant:
            constant = true;
            return true;
        case PropertyType::Tags:
            tags = values;
            return true;
        case PropertyType::Condition:
            condition = values.at(0);
            return true;
            // metadata
        case PropertyType::Description:
            metadata.description += values.at(0);
            return true;
        case PropertyType::Authors:
            metadata.authors += values.at(0);
            return true;
        case PropertyType::Title:
            metadata.title += values.at(0);
            return true;
        case PropertyType::Journal:
            metadata.journal += values.at(0);
            return true;
        case PropertyType::Year:
            metadata.year += values.at(0);
            return true;
        case PropertyType::Volume:
            metadata.volume += values.at(0);
            return true;
        case PropertyType::Issue:
            metadata.issue += values.at(0);
            return true;
        case PropertyType::Pages:
            metadata.pages += values.at(0);
            return true;
        case PropertyType::DOI:
            metadata.doi += values.at(0);
            return true;
        case PropertyType::URL:
            metadata.url += values.at(0);
            return true;
        case PropertyType::Version:
            metadata.version += values.at(0);
            return true;
        case PropertyType::Created:
            metadata.created += values.at(0);
            return true;
        case PropertyType::Modified:
            metadata.modified += values.at(0);
            return true;
        case PropertyType::License:
            metadata.license += values.at(0);
            return true;
        default:
            return false;
        }
    }

    /*
     * Validation of node properties and values
     */

    void ValueNode::validate_constant() const {
        if (constant)
            throw dip::SyntaxException(
                "Constant node",
                "The node `" + path.name + "` is constant and cannot be modified.",
                "Remove the `constant` property or modify a different node.",
                __FILE__,
                __LINE__,
                line
            );
    }

    void ValueNode::validate_definition() const {
        if (value == nullptr && value_origin != ValueOrigin::None)
            throw dip::SyntaxException(
                "Undefined value",
                "The node has a value origin but no value has been defined.",
                "Provide a valid value for the declared node.",
                __FILE__,
                __LINE__,
                line
            );
    }

    void ValueNode::validate_condition(Environment& env) const {
        bool passed = true;
        if (!condition.empty()) {
            if (condition == core::KEYWORD_FALSE) {
                passed = false;
            } else if (condition == core::KEYWORD_TRUE) {
                return;
            } else {
                LogicalSolver solver(env, path);
                ValueNodeData data = solver.eval(condition);
                if (!data.value->all_of())
                    passed = false;
            }
        }
        if (!passed)
            throw dip::SyntaxException(
                "Node does not satisfy its condition",
                "The condition `" + condition + "` does not pass.",
                "Check the condition values and references.",
                __FILE__,
                __LINE__,
                this->line
            );
    }

    void ValueNode::validate_options() const {
        if (options.size() > 0) {
            bool match = false;
            for (const auto& option : options) {
                if (option.value && value->compare_equal(option.value.get())->all_of())
                    match = true;
            }
            if (!match) {
                std::ostringstream oss;
                for (int i = 0; i < options.size(); i++) {
                    if (i > 0)
                        oss << ", ";
                    oss << options[i].value->to_string();
                }
                throw dip::SyntaxException(
                    "Invalid option",
                    "The value `" + value->to_string() + "` does not match any of the defined options: `" + oss.str() +
                        "`.",
                    "Use one of the values defined by the node's `options` property.",
                    __FILE__,
                    __LINE__,
                    line
                );
            }
        }
    }

    void ValueNode::validate_format() const {
        if (format.size() > 0)
            throw dip::SyntaxException(
                "Invalid property",
                "The `format` property is only supported for string nodes.",
                "Use the `format` property only with a string node.",
                __FILE__,
                __LINE__,
                line
            );
    }

    void ValueNode::validate_dimensions() const {
        val::Array::ShapeType vdim;
        if (value == nullptr) {
            vdim.assign(dimension.size(), 0);
        } else {
            vdim = value->get_shape();
            if (vdim.size() == 1 && vdim[0] == 0) {
                vdim.assign(dimension.size(), 0);
            } else if (dimension.size() != vdim.size()) {
                throw dip::SyntaxException(
                    "Dimension mismatch",
                    "The value has " + std::to_string(vdim.size()) + " dimensions, but the node has " +
                        std::to_string(dimension.size()) + " dimensions.",
                    "Ensure that the value and node have the same number of dimensions.",
                    __FILE__,
                    __LINE__,
                    line
                );
            }
        }
        for (size_t i = 0; i < dimension.size(); i++) {
            size_t dmin = dimension[i].dmin;
            size_t dmax = dimension[i].dmax; // dimension ranges can be max(size_t)
            if (dmax == val::Array::max_range)
                dmax = vdim[i];
            if (vdim[i] < dmin || dmax < vdim[i]) {
                std::ostringstream nss, vss;
                for (size_t j = 0; j < dimension.size(); j++) {
                    if (j > 0) {
                        nss << ",";
                        vss << ",";
                    }
                    dmin = dimension[j].dmin;
                    dmax = dimension[j].dmax;
                    if (dmin == 0 && dmax == val::Array::max_range)
                        nss << SEPARATOR_SLICE;
                    else if (dmin == dmax)
                        nss << dmin;
                    else if (dmax == val::Array::max_range)
                        nss << dmin << SEPARATOR_SLICE;
                    else if (dmin == 0)
                        nss << SEPARATOR_SLICE << dmax;
                    else
                        nss << dmin << SEPARATOR_SLICE << dmax;
                    vss << vdim[j];
                }
                throw dip::SyntaxException(
                    "Dimension range mismatch",
                    "The value dimensions `[" + vss.str() + "]` do not correspond to the node dimension ranges `[" +
                        nss.str() + "]`.",
                    "Ensure that each value dimension falls within the corresponding dimension range of the node.",
                    __FILE__,
                    __LINE__,
                    line
                );
            }
        }
        // std::cout << "checking dimensions " << std::endl;
        // std::cout << dimension.size() << std::endl;
        // std::cout << value->dimension().size() << std::endl;
    }

} // namespace snt::dip
