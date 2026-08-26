#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_boolean.h>
#include <snt/dip/nodes/node_float.h>
#include <snt/dip/nodes/node_integer.h>
#include <snt/dip/nodes/node_string.h>
#include <snt/dip/solvers/numerical_atom.h>

namespace snt::dip {

    NumericalAtom& NumericalAtom::operator=(const NumericalAtom& a) {
        if (this != &a) {
            value.value = a.value.value->clone();
            if (value.units)
                value.units = a.value.units;
        }
        return *this;
    }

    NumericalAtom::NumericalAtom(const NumericalAtom& a)
        : AtomBase({(a.value.value) ? a.value.value->clone() : nullptr, a.value.units}) {};

    ValueNodeData NumericalAtom::from_string(const std::string& s, exs::BaseSettings* settings) {
        Parser parser({s, {"NUMERICAL_ATOM", 0}});
        NumericalSettings* csettings = static_cast<NumericalSettings*>(settings);
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
                    "The value type could not be determined from the string `" + s + "`.",
                    "Use a valid boolean, integer, float, or string value.",
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
                "The string `" + s + "` is neither a valid reference nor a literal value.",
                "Check whether the syntax is correct.",
                __FILE__,
                __LINE__
            );
        }
    }

    std::string NumericalAtom::to_string() {
        if (value.units)
            return value.value->to_string() + " " + value.units->to_string();
        else
            return value.value->to_string();
    }

    // Mathematical operations
    void NumericalAtom::math_sinus() {
        if (!value.value) {
            throw dip::SolverException(
                "Undefined numerical atom value",
                "The numerical atom does not have a defined value.",
                "Define the numerical atom before applying the sine operation.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->math_sin();
        }
    }
    void NumericalAtom::math_cosinus() {
        if (!value.value) {
            throw dip::SolverException(
                "Undefined numerical atom value",
                "The numerical atom does not have a defined value.",
                "Define the numerical atom before applying the cosine operation.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->math_cos();
        }
    }
    void NumericalAtom::math_tangens() {
        if (!value.value) {
            throw dip::SolverException(
                "Undefined numerical atom value",
                "The numerical atom does not have a defined value.",
                "Define the numerical atom before applying the tangens operation.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->math_tan();
        }
    }
    void NumericalAtom::math_cubic_root() {
        if (!value.value) {
            throw dip::SolverException(
                "Undefined numerical atom value",
                "The numerical atom does not have a defined value.",
                "Define the numerical atom before applying the cubic root operation.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->math_cbrt();
        }
    }
    void NumericalAtom::math_square_root() {
        if (!value.value) {
            throw dip::SolverException(
                "Undefined numerical atom value",
                "The numerical atom does not have a defined value.",
                "Define the numerical atom before applying the square root operation.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->math_sqrt();
        }
    }
    // void NumericalAtom::math_power_base(NumericalAtom* other) {
    //   value.value = value.value->math_powb(other->value.get());
    // }
    // void NumericalAtom::math_logarithm_base(NumericalAtom* other) {
    //   value.value = value.value->math_logb(other->value.get());
    // }
    void NumericalAtom::math_logarithm_10() {
        if (!value.value) {
            throw dip::SolverException(
                "Undefined numerical atom value",
                "The numerical atom does not have a defined value.",
                "Define the numerical atom before applying the decadic logarithm operation.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->math_log10();
        }
    }
    void NumericalAtom::math_logarithm() {
        if (!value.value) {
            throw dip::SolverException(
                "Undefined numerical atom value",
                "The numerical atom does not have a defined value.",
                "Define the numerical atom before applying the natural logarithm operation.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->math_log();
        }
    }
    void NumericalAtom::math_exponent() {
        if (!value.value) {
            throw dip::SolverException(
                "Undefined numerical atom value",
                "The numerical atom does not have a defined value.",
                "Define the numerical atom before applying the exponent operation.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->math_exp();
        }
    }
    void NumericalAtom::math_power(NumericalAtom* other) {
        if (!value.value || !other->value.value) {
            throw dip::SolverException(
                "Undefined numerical atom value",
                "The numerical atom does not have a defined value.",
                "Define the numerical atom before applying the power operation.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->math_pow(other->value.value.get());
        }
    }
    void NumericalAtom::math_multiply(NumericalAtom* other) {
        if (!value.value || !other->value.value) {
            throw dip::SolverException(
                "Undefined numerical atom value",
                "The numerical atom does not have a defined value.",
                "Define the numerical atom before applying the multiplication operation.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->math_mul(other->value.value.get());
        }
        if (value.units && other->value.units) {
            value.units = (*value.units) * (*other->value.units);
        } else if (other->value.units) {
            value.units = other->value.units;
        }
    }
    void NumericalAtom::math_divide(NumericalAtom* other) {
        if (!value.value || !other->value.value) {
            throw dip::SolverException(
                "Undefined numerical atom value",
                "The numerical atom does not have a defined value.",
                "Define the numerical atom before applying the division operation.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->math_div(other->value.value.get());
        }
        if (value.units && other->value.units) {
            value.units = (*value.units) / (*other->value.units);
        } else if (other->value.units) {
            value.units = 1. / (*other->value.units);
        }
    }
    void NumericalAtom::math_add(NumericalAtom* other) {
        if (!value.value || !other->value.value) {
            throw dip::SolverException(
                "Undefined numerical atom value",
                "The numerical atom does not have a defined value.",
                "Define the numerical atom before applying the addition operation.",
                __FILE__,
                __LINE__
            );
        } else if (value.units && other->value.units) {
            puq::Quantity quantity = std::move(other->value.value) * (*other->value.units);
            quantity = quantity.convert(*value.units);
            val::BaseValue::PointerType new_value = std::move(quantity.measurement.result.estimate);
            value.value = value.value->math_add(new_value.get());
        } else if (value.units) {
            throw dip::UnitException(
                "Dimension mismatch",
                "A nondimensional quantity cannot be added to a quantity with dimensions `" + value.units->to_string() +
                    "`.",
                "Check whether the input units are correct.",
                __FILE__,
                __LINE__
            );
        } else if (other->value.units) {
            throw dip::UnitException(
                "Dimension mismatch",
                "A quantity with dimensions `" + other->value.units->to_string() +
                    "` cannot be added to a nondimensional quantity.",
                "Check whether the input units are correct.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->math_add(other->value.value.get());
        }
    }
    void NumericalAtom::math_subtract(NumericalAtom* other) {
        if (!value.value || !other->value.value) {
            throw dip::SolverException(
                "Undefined numerical atom value",
                "The numerical atom does not have a defined value.",
                "Define the numerical atom before applying the subtraction operation.",
                __FILE__,
                __LINE__
            );
        } else if (value.units && other->value.units) {
            puq::Quantity quantity = std::move(other->value.value) * (*other->value.units);
            quantity = quantity.convert(*value.units);
            val::BaseValue::PointerType new_value = std::move(quantity.measurement.result.estimate);
            value.value = value.value->math_sub(new_value.get());
        } else if (value.units) {
            throw dip::UnitException(
                "Dimension mismatch",
                "A nondimensional quantity cannot be subtracted from a quantity with dimensions `" +
                    value.units->to_string() + "`.",
                "Check whether the input units are correct.",
                __FILE__,
                __LINE__
            );
        } else if (other->value.units) {
            throw dip::UnitException(
                "Dimension mismatch",
                "A quantity with dimensions `" + other->value.units->to_string() +
                    "` cannot be subtracted from a nondimensional quantity.",
                "Check whether the input units are correct.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->math_sub(other->value.value.get());
        }
    }
    void NumericalAtom::math_negate() {
        if (!value.value) {
            throw dip::SolverException(
                "Undefined numerical atom value",
                "The numerical atom does not have a defined value.",
                "Define the numerical atom before applying the negation operation.",
                __FILE__,
                __LINE__
            );
        } else {
            value.value = value.value->math_neg();
        }
    }

} // namespace snt::dip
