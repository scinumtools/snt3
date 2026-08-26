#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_boolean.h>
#include <snt/dip/nodes/node_float.h>
#include <snt/dip/nodes/node_integer.h>
#include <snt/dip/nodes/node_string.h>
#include <snt/dip/solvers/logical_atom.h>

namespace snt::dip {

    LogicalAtom& LogicalAtom::operator=(const LogicalAtom& a) {
        if (this != &a) {
            value.value = a.value.value->clone();
            if (value.units)
                value.units = a.value.units;
        }
        return *this;
    }

    ValueNodeData LogicalAtom::from_string(const std::string& s, exs::BaseSettings* settings) {
        Parser parser({s, {"LOGICAL_ATOM", 0}});
        LogicalSettings* csettings = static_cast<LogicalSettings*>(settings);
        if (parser.part_reference()) {
            std::string request = parser.value_raw.at(0);
            // resolve relative path
            if (!request.empty() && request[0] == SIGN_SEPARATOR)
                request = std::string(1, SIGN_QUERY) + csettings->current.resolve(request).name;
            // request absolute path
            return csettings->env->request_node_data(request, RequestType::Reference);
        } else if (parser.part_literal()) {
            ValueNode::PointerType vnode = nullptr;
            if (vnode == nullptr)
                vnode = BooleanNode::is_node(parser);
            if (vnode == nullptr)
                vnode = IntegerNode::is_node(parser);
            if (vnode == nullptr)
                vnode = FloatNode::is_node(parser);
            if (vnode == nullptr)
                vnode = StringNode::is_node(parser);
            if (vnode == nullptr)
                throw dip::SolverException(
                    "Could not determine the value type from the string",
                    "The string `" + s + "` could not be parsed as a boolean, integer, float, or string value.",
                    "Check whether the syntax is correct.",
                    __FILE__,
                    __LINE__
                );
            vnode->set_value();
            ValueNodeData data;
            data.value = std::move(vnode->value);
            if (!vnode->units_raw.empty())
                data.units = puq::Quantity(vnode->units_raw);
            return data;
        } else {
            throw dip::SolverException(
                "Could not parse value from a string",
                "The string `" + s + "` could not be parsed as a reference or literal value.",
                "Check whether the syntax is correct.",
                __FILE__,
                __LINE__
            );
        }
    }

    std::string LogicalAtom::to_string() {
        return value.value->to_string();
    }

    // Comparison operations
    void LogicalAtom::comparison_equal(LogicalAtom* other) {
        if (!value.value || !other->value.value) {
            throw dip::SolverException(
                "Undefined logical atom value",
                "One or both operands in `==` do not have a defined value.",
                "Define both operands before performing the comparison.",
                __FILE__,
                __LINE__
            );
        } else if (value.units && other->value.units) {
            puq::Quantity quantity = std::move(other->value.value) * (*other->value.units);
            quantity = quantity.convert(*value.units);
            val::BaseValue::PointerType new_value = std::move(quantity.measurement.result.estimate);
            value.value = value.value->compare_equal(new_value.get());
        } else if (value.units) {
            throw dip::UnitException(
                "Dimension mismatch",
                "The `==` operator cannot compare a nondimensional quantity with a quantity having dimensions `" +
                    other->value.units->to_string() + "`.",
                "Check whether the input units are correct.",
                __FILE__,
                __LINE__
            );
        } else if (other->value.units) {
            throw dip::UnitException(
                "Dimension mismatch",
                "The `==` operator cannot compare a quantity with dimensions `" + value.units->to_string() +
                    "` with a nondimensional quantity.",
                "Check whether the input units are correct.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->compare_equal(other->value.value.get());
        }
        value.units = std::nullopt;
    }

    void LogicalAtom::comparison_not_equal(LogicalAtom* other) {
        if (!value.value || !other->value.value) {
            throw dip::SolverException(
                "Undefined logical atom value",
                "One or both operands in `!=` do not have a defined value.",
                "Define both operands before performing the comparison.",
                __FILE__,
                __LINE__
            );
        } else if (value.units && other->value.units) {
            puq::Quantity quantity = std::move(other->value.value) * (*other->value.units);
            quantity = quantity.convert(*value.units);
            val::BaseValue::PointerType new_value = std::move(quantity.measurement.result.estimate);
            value.value = value.value->compare_not_equal(new_value.get());
        } else if (value.units) {
            throw dip::UnitException(
                "Dimension mismatch",
                "The `!=` operator cannot compare a nondimensional quantity with a quantity having dimensions `" +
                    other->value.units->to_string() + "`.",
                "Check whether the input units are correct.",
                __FILE__,
                __LINE__
            );
        } else if (other->value.units) {
            throw dip::UnitException(
                "Dimension mismatch",
                "The `!=` operator cannot compare a quantity with dimensions `" + value.units->to_string() +
                    "` with a nondimensional quantity.",
                "Check whether the input units are correct.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->compare_not_equal(other->value.value.get());
        }
        value.units = std::nullopt;
    }

    void LogicalAtom::comparison_less_equal(LogicalAtom* other) {
        if (!value.value || !other->value.value) {
            throw dip::SolverException(
                "Undefined logical atom value",
                "One or both operands in `<=` do not have a defined value.",
                "Define both operands before performing the comparison.",
                __FILE__,
                __LINE__
            );
        } else if (value.units && other->value.units) {
            puq::Quantity quantity = std::move(other->value.value) * (*other->value.units);
            quantity = quantity.convert(*value.units);
            val::BaseValue::PointerType new_value = std::move(quantity.measurement.result.estimate);
            value.value = value.value->compare_less_equal(new_value.get());
        } else if (value.units) {
            throw dip::UnitException(
                "Dimension mismatch",
                "The `<=` operator cannot compare a nondimensional quantity with a quantity having dimensions `" +
                    other->value.units->to_string() + "`.",
                "Check whether the input units are correct.",
                __FILE__,
                __LINE__
            );
        } else if (other->value.units) {
            throw dip::UnitException(
                "Dimension mismatch",
                "The `<=` operator cannot compare a quantity with dimensions `" + value.units->to_string() +
                    "` with a nondimensional quantity.",
                "Check whether the input units are correct.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->compare_less_equal(other->value.value.get());
        }
        value.units = std::nullopt;
    }

    void LogicalAtom::comparison_greater_equal(LogicalAtom* other) {
        if (!value.value || !other->value.value) {
            throw dip::SolverException(
                "Undefined logical atom value",
                "One or both operands in `>=` do not have a defined value.",
                "Define both operands before performing the comparison.",
                __FILE__,
                __LINE__
            );
        } else if (value.units && other->value.units) {
            puq::Quantity quantity = std::move(other->value.value) * (*other->value.units);
            quantity = quantity.convert(*value.units);
            val::BaseValue::PointerType new_value = std::move(quantity.measurement.result.estimate);
            value.value = value.value->compare_greater_equal(new_value.get());
        } else if (value.units) {
            throw dip::UnitException(
                "Dimension mismatch",
                "The `>=` operator cannot compare a nondimensional quantity with a quantity having dimensions `" +
                    other->value.units->to_string() + "`.",
                "Check whether the input units are correct.",
                __FILE__,
                __LINE__
            );
        } else if (other->value.units) {
            throw dip::UnitException(
                "Dimension mismatch",
                "The `>=` operator cannot compare a quantity with dimensions `" + value.units->to_string() +
                    "` with a nondimensional quantity.",
                "Check whether the input units are correct.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->compare_greater_equal(other->value.value.get());
        }
        value.units = std::nullopt;
    }

    void LogicalAtom::comparison_less(LogicalAtom* other) {
        if (!value.value || !other->value.value) {
            throw dip::SolverException(
                "Undefined logical atom value",
                "One or both operands in `<` do not have a defined value.",
                "Define both operands before performing the comparison.",
                __FILE__,
                __LINE__
            );
        } else if (value.units && other->value.units) {
            puq::Quantity quantity = std::move(other->value.value) * (*other->value.units);
            quantity = quantity.convert(*value.units);
            val::BaseValue::PointerType new_value = std::move(quantity.measurement.result.estimate);
            value.value = value.value->compare_less(new_value.get());
        } else if (value.units) {
            throw dip::UnitException(
                "Dimension mismatch",
                "The `<` operator cannot compare a nondimensional quantity with a quantity having dimensions `" +
                    other->value.units->to_string() + "`.",
                "Check whether the input units are correct.",
                __FILE__,
                __LINE__
            );
        } else if (other->value.units) {
            throw dip::UnitException(
                "Dimension mismatch",
                "The `<` operator cannot compare a quantity with dimensions `" + value.units->to_string() +
                    "` with a nondimensional quantity.",
                "Check whether the input units are correct.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->compare_less(other->value.value.get());
        }
        value.units = std::nullopt;
    }

    void LogicalAtom::comparison_greater(LogicalAtom* other) {
        if (!value.value || !other->value.value) {
            throw dip::SolverException(
                "Undefined logical atom value",
                "One or both operands in `>` do not have a defined value.",
                "Define both operands before performing the comparison.",
                __FILE__,
                __LINE__
            );
        } else if (value.units && other->value.units) {
            puq::Quantity quantity = std::move(other->value.value) * (*other->value.units);
            quantity = quantity.convert(*value.units);
            val::BaseValue::PointerType new_value = std::move(quantity.measurement.result.estimate);
            value.value = value.value->compare_greater(new_value.get());
        } else if (value.units) {
            throw dip::UnitException(
                "Dimension mismatch",
                "The `>` operator cannot compare a nondimensional quantity with a quantity having dimensions `" +
                    other->value.units->to_string() + "`.",
                "Check whether the input units are correct.",
                __FILE__,
                __LINE__
            );
        } else if (other->value.units) {
            throw dip::UnitException(
                "Dimension mismatch",
                "The `>` operator cannot compare a quantity with dimensions `" + value.units->to_string() +
                    "` with a nondimensional quantity.",
                "Check whether the input units are correct.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->compare_greater(other->value.value.get());
        }
        value.units = std::nullopt;
    }

    // Logical operations
    void LogicalAtom::logical_not() {
        if (!value.value) {
            throw dip::SolverException(
                "Undefined logical atom value",
                "The operand in `~` does not have a defined value.",
                "Define the logical atom before applying the logical NOT operation.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->logical_not();
        }
    }
    void LogicalAtom::logical_and(LogicalAtom* other) {
        if (!value.value) {
            throw dip::SolverException(
                "Undefined logical atom value",
                "The operand in `&&` does not have a defined value.",
                "Define the logical atom before applying the logical AND operation.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->logical_and(other->value.value.get());
        }
    }
    void LogicalAtom::logical_or(LogicalAtom* other) {
        if (!value.value) {
            throw dip::SolverException(
                "Undefined logical atom value",
                "The operand in `||` does not have a defined value.",
                "Define the logical atom before applying the logical OR operation.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->logical_or(other->value.value.get());
        }
    }

    // Definition operators
    void LogicalAtom::custom_defined() {
        value.value = std::make_unique<val::ArrayValueBool>(value.value != nullptr);
    }
    void LogicalAtom::custom_not_defined() {
        value.value = std::make_unique<val::ArrayValueBool>(value.value == nullptr);
    }

} // namespace snt::dip
