#include <snt/dip/nodes/node_integer.h>
#include <optional>
#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_value.h>
#include <snt/dip/parsers.h>
#include <snt/dip/solvers/logical_solver.h>
#include <snt/dip/solvers/numerical_solver.h>
#include <snt/dip/solvers/template_solver.h>
#include <sstream>

namespace snt::dip {

    bool ValueMetadata::set_property(PropertyType property, const val::Array::StringType& values) {
        switch (property) {
#define SNT_METADATA_SCALAR(TYPE, FIELD)                                                                                \
    case PropertyType::TYPE:                                                                                             \
        FIELD += values.at(0);                                                                                           \
        return true
#define SNT_METADATA_LIST(TYPE, FIELD)                                                                                  \
    case PropertyType::TYPE:                                                                                             \
        FIELD.insert(FIELD.end(), values.begin(), values.end());                                                         \
        return true
            SNT_METADATA_SCALAR(Description, description);
            SNT_METADATA_SCALAR(Authors, authors);
            SNT_METADATA_SCALAR(Title, title);
            SNT_METADATA_SCALAR(Journal, journal);
            SNT_METADATA_SCALAR(Year, year);
            SNT_METADATA_SCALAR(Volume, volume);
            SNT_METADATA_SCALAR(Issue, issue);
            SNT_METADATA_SCALAR(Pages, pages);
            SNT_METADATA_SCALAR(DOI, doi);
            SNT_METADATA_SCALAR(URL, url);
            SNT_METADATA_SCALAR(Version, version);
            SNT_METADATA_SCALAR(Created, created);
            SNT_METADATA_SCALAR(Modified, modified);
            SNT_METADATA_SCALAR(License, license);
            SNT_METADATA_SCALAR(Rationale, rationale);
            SNT_METADATA_LIST(Native, native);
            SNT_METADATA_LIST(Requires, requires);
            SNT_METADATA_LIST(Conflicts, conflicts);
            SNT_METADATA_LIST(Implies, implies);
            SNT_METADATA_LIST(See, see);
            SNT_METADATA_LIST(Example, example);
            SNT_METADATA_SCALAR(RecommendedRange, recommended_range);
            SNT_METADATA_SCALAR(PerformanceImpact, performance_impact);
            SNT_METADATA_SCALAR(ScientificImpact, scientific_impact);
            SNT_METADATA_SCALAR(Deprecated, deprecated);
            SNT_METADATA_SCALAR(Replacement, replacement);
            SNT_METADATA_SCALAR(Since, since);
            SNT_METADATA_SCALAR(Category, category);
            SNT_METADATA_SCALAR(Visibility, visibility);
#undef SNT_METADATA_SCALAR
#undef SNT_METADATA_LIST
        default:
            return false;
        }
    }

    ValueNode::ValueNode(const ValueNode& other)
        : units(other.units), tags(other.tags), constant(other.constant), override(other.override),
          override_line(other.override_line), modification_lines(other.modification_lines), table_path(other.table_path),
          copied_from(other.copied_from),
          table_column_index(other.table_column_index), metadata(other.metadata),
          condition(other.condition), format(other.format), value_dtype(other.value_dtype), BaseNode(other) {
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
        const Path& pth, val::BaseValue::PointerType val, const NodeDtype dt,
        std::optional<puq::Quantity> unt
    )
        : constant(false), value_dtype(val->get_dtype()), units(unt), BaseNode(dt) {
        path = pth;
        val::Array::ShapeType dims = val->get_shape();
        if (val->get_size() > 1) {
            dimension.clear();
            dimension.reserve(dims.size());
            for (size_t dim : dims) {
                dimension.push_back({dim, dim});
            }
        }
        set_value(std::move(val));
        if (unt)
            set_units(unt);
    };

    ValueNode::ValueNode(const BaseNode::PointerType other, const NodeDtype dt, const core::DataType vdt)
        : constant(false), value_dtype(vdt) {
        dtype = dt;
        path = other->path;
        indent = other->indent;
        line = other->line;
        schema_id = other->schema_id;
    }

    ValueNode::ValueNode(const ValueNode::PointerType other, const NodeDtype dt)
        : constant(false), value_dtype(other->value_dtype) {
        dtype = dt;
        path = other->path;
        indent = other->indent;
        line = other->line;
        schema_id = other->schema_id;
    }

    val::BaseValue::PointerType ValueNode::parse_function(
        Environment& env, const std::string& name, std::optional<std::string_view> units
    ) const {
        return env.request_value(name, RequestType::Function, units);
    }

    val::BaseValue::PointerType ValueNode::parse_reference(
        Environment& env, std::string query, std::optional<std::string_view> units, ValueOrigin origin
    ) const {
        switch (origin) {
        case ValueOrigin::Reference:
        case ValueOrigin::ReferenceRel: {
            if (origin == ValueOrigin::ReferenceRel) {
                Path current = env.hierarchy.get_current_path(indent, path.name);
                query = std::string(1, SIGN_QUERY) + current.resolve(query).name;
            }
            val::BaseValue::PointerType val = env.request_value(query, RequestType::Reference, units);
            if (val)
                return std::move(val);
            else
                throw dip::SyntaxException(
                    "Undefined reference",
                    "The requested value reference `" + query + "` does not resolve to a value.",
                    "Ensure that the referenced node exists and has a defined value.",
                    __FILE__,
                    __LINE__,
                    line
                );
        }
        case ValueOrigin::ReferenceRaw: {
            std::string source_code = env.request_code(query);
            val::Array::StringType source_value_raw;
            val::Array::ShapeType source_value_shape;
            parse_value(source_code, source_value_raw, source_value_shape);
            return cast_value(source_value_raw, source_value_shape);
        }
        default:
            throw dip::SyntaxException(
                "Unknown value origin",
                "Node value origin is not a reference: `" + ValueOriginNames.at(origin) + "`.",
                "Ensure that the node value origin is either an absolute, relative, or raw reference.",
                __FILE__,
                __LINE__,
                line
            );
        }
    }

    val::BaseValue::PointerType ValueNode::parse_expression(
        Environment& env, const std::string& expression, std::optional<std::string_view> units, const NodeDtype ntype
    ) const {
        Path current_path = env.hierarchy.get_current_path(indent, path.name);
        switch (ntype) {
        case NodeDtype::Integer:
        case NodeDtype::Float: {
            NumericalSolver solver(env, current_path);
            ValueNodeData data = solver.eval(expression, std::string(units ? units.value() : ""));
            return std::move(data.value);
        }
        case NodeDtype::Boolean: {
            LogicalSolver solver(env, current_path);
            ValueNodeData data = solver.eval(expression);
            return std::move(data.value);
        }
        case NodeDtype::String: {
            TemplateSolver solver(env, current_path);
            ValueNodeData data = solver.eval(expression);
            return std::move(data.value);
        }
        default:
            throw dip::SyntaxException(
                "No expression solver available",
                "No expression solver is available for the current node type: `" + NodeDtypeNames.at(ntype) + "`.",
                "Expression solving is supported only for boolean, integer, float, and string nodes.",
                __FILE__,
                __LINE__,
                line
            );
        }
    }

    val::BaseValue::PointerType ValueNode::cast_value() const {
        return cast_value(value_raw, value_shape);
    }

    val::BaseValue::PointerType ValueNode::cast_value(
        const val::Array::StringType& value_input, const val::Array::ShapeType& shape
    ) const {
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
        // Keep declarations unassigned until instance modifications or final validation.
        if (value_input == nullptr && value_origin == ValueOrigin::Empty && value_raw.empty())
            return;
        if (value_input == nullptr && !value_raw.empty() && !value_raw.at(0).empty()) {
            // Triple-quoted values are initially parsed as a single string. When
            // an array dimension is declared, parse an array literal contained
            // in that string before converting it to the node's value type.
            if (value_origin == ValueOrigin::String && !dimension.empty() && value_shape.empty()) {
                val::Array::StringType parsed_values;
                val::Array::ShapeType parsed_shape;
                parse_value(value_raw.at(0), parsed_values, parsed_shape);
                value = cast_value(parsed_values, parsed_shape);
            } else {
                value = cast_value();
            }
        } else if (value_input == nullptr && value_origin == ValueOrigin::Array) {
            value = cast_value();
        } else if (value_input != nullptr) {
            if (dtype == NodeDtype::Integer)
                IntegerNode::validate_value(value_input.get(), value_dtype, line);
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

    void ValueNode::validate_modification(const BaseNode::PointerType& node) const {
        // check if modification and target node have the same node type
        if (node->dtype != NodeDtype::Modification && node->dtype != dtype)
            throw dip::SyntaxException(
                "Type mismatch",
                "A node of type `" + NodeDtypeNames.at(dtype) + "` cannot modify a node of type `" +
                    NodeDtypeNames.at(node->dtype) + "`.",
                "Use a modification node or a node with a compatible type.",
                __FILE__,
                __LINE__,
                line
            );
    }

    void ValueNode::apply_override(const BaseNode::PointerType& node, Environment& env) {
        if ((dtype == NodeDtype::Boolean || dtype == NodeDtype::String) && !units_raw.empty())
            throw dip::UnitException(
                "Invalid units", "The declared value type does not support units.",
                "Remove units from the original declaration.", __FILE__, __LINE__, line
            );
        if (node->value_origin == ValueOrigin::None && !node->units_raw.empty())
            throw dip::UnitException(
                "Invalid null override units", "A null value cannot specify replacement units.",
                "Use `path = none` without units.", __FILE__, __LINE__, node->line
            );
        set_units();
        auto evaluated = std::dynamic_pointer_cast<ValueNode>(clone(path, indent));
        evaluated->value_dtype = value_dtype;
        evaluated->value.reset();
        evaluated->units.reset();
        evaluated->options.clear();
        evaluated->line = node->line;
        evaluated->value_origin = node->value_origin;
        evaluated->value_raw = node->value_raw;
        evaluated->value_shape = node->value_shape;
        evaluated->value_slice = node->value_slice;
        if (!node->units_raw.empty())
            evaluated->units_raw = node->units_raw;
        evaluated->parse(env);
        if (evaluated->units) {
            if (!units)
                throw dip::UnitException(
                    "Dimension mismatch", "An override cannot add units to a nondimensional node.",
                    "Use units compatible with the target declaration.", __FILE__, __LINE__, node->line
                );
            if (evaluated->value) {
                puq::Quantity quantity(std::move(evaluated->value), evaluated->units->to_string());
                quantity = quantity.convert(*units);
                evaluated->value = std::move(quantity.measurement.result.estimate);
            }
        }
        if (dtype == NodeDtype::Integer)
            IntegerNode::validate_value(evaluated->value.get(), value_dtype, node->line);
        if (evaluated->value && evaluated->value->get_dtype() != value_dtype)
            evaluated->value = evaluated->value->cast_as(value_dtype);
        value = std::move(evaluated->value);
        value_origin = node->value_origin;
        value_raw = node->value_raw;
        value_shape = node->value_shape;
        value_slice = node->value_slice;
        override = true;
        override_line = node->line;
    }

    void ValueNode::modify_value(const BaseNode::PointerType& node, Environment& env) {
        validate_modification(node);
        // Nested schema expansion can repeat a declaration before its value is assigned.
        if (node->dtype != NodeDtype::Modification && node->value_origin == ValueOrigin::Empty &&
            node->value_raw.empty())
            return;
        // parse value from raw data
        std::optional<std::string_view> units;
        if (dtype == NodeDtype::Integer || dtype == NodeDtype::Float)
            units = node->units_raw;
        val::BaseValue::PointerType value;
        switch (node->value_origin) {
        case ValueOrigin::Function:
            value = parse_function(env, node->value_raw.at(0), units);
            break;
        case ValueOrigin::Reference:
        case ValueOrigin::ReferenceRel:
        case ValueOrigin::ReferenceRaw:
            value = parse_reference(env, node->value_raw.at(0), units, node->value_origin);
            break;
        case ValueOrigin::Expression: {
            value = parse_expression(env, node->value_raw.at(0), units, dtype);
            break;
        }
        default:
            value = cast_value(node->value_raw, node->value_shape);
            break;
        }
        // convert units if necessary
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
        if (env.block_input_recording()) env.erase_block_input(path.name);
        modification_lines.push_back(node->line);
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
        case PropertyType::Authors:
        case PropertyType::Title:
        case PropertyType::Journal:
        case PropertyType::Year:
        case PropertyType::Volume:
        case PropertyType::Issue:
        case PropertyType::Pages:
        case PropertyType::DOI:
        case PropertyType::URL:
        case PropertyType::Version:
        case PropertyType::Created:
        case PropertyType::Modified:
        case PropertyType::License:
        case PropertyType::Rationale:
        case PropertyType::Native:
        case PropertyType::Requires:
        case PropertyType::Conflicts:
        case PropertyType::Implies:
        case PropertyType::See:
        case PropertyType::Example:
        case PropertyType::RecommendedRange:
        case PropertyType::PerformanceImpact:
        case PropertyType::ScientificImpact:
        case PropertyType::Deprecated:
        case PropertyType::Replacement:
        case PropertyType::Since:
        case PropertyType::Category:
        case PropertyType::Visibility:
            return metadata.set_property(property, values);
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
