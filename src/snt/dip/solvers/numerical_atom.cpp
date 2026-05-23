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

    ValueNodeData NumericalAtom::from_string(const std::string& s, exs::BaseSettings* settings) {
        Parser parser({s, {"NUMERICAL_ATOM", 0}});
        NumericalSettings* csettings = static_cast<NumericalSettings*>(settings);
        ValueNodeData data;
        if (parser.part_reference()) {
            return csettings->env->request_node_data(parser.value_raw.at(0), RequestType::Reference);
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
                throw std::runtime_error("Value could not be determined from : " + s);
            vnode->set_value();
            data.value = std::move(vnode->value);
            if (!vnode->units_raw.empty())
                data.units = puq::Quantity(vnode->units_raw);
            return data;
        } else {
            throw std::runtime_error("Invalid atom value: " + s);
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
        value.value = value.value->math_sin();
    }
    void NumericalAtom::math_cosinus() {
        value.value = value.value->math_cos();
    }
    void NumericalAtom::math_tangens() {
        value.value = value.value->math_tan();
    }
    void NumericalAtom::math_cubic_root() {
        value.value = value.value->math_cbrt();
    }
    void NumericalAtom::math_square_root() {
        value.value = value.value->math_sqrt();
    }
    // void NumericalAtom::math_power_base(NumericalAtom* other) {
    //   value.value = value.value->math_powb(other->value.get());
    // }
    // void NumericalAtom::math_logarithm_base(NumericalAtom* other) {
    //   value.value = value.value->math_logb(other->value.get());
    // }
    void NumericalAtom::math_logarithm_10() {
        value.value = value.value->math_log10();
    }
    void NumericalAtom::math_logarithm() {
        value.value = value.value->math_log();
    }
    void NumericalAtom::math_exponent() {
        value.value = value.value->math_exp();
    }
    void NumericalAtom::math_power(NumericalAtom* other) {
        value.value = value.value->math_pow(other->value.value.get());
    }
    void NumericalAtom::math_multiply(NumericalAtom* other) {
        value.value = value.value->math_mul(other->value.value.get());
        if (value.units && other->value.units) {
	  value.units = (*value.units) * (*other->value.units);	  
        } else if (other->value.units) {
	  value.units = other->value.units;
        }	
    }
    void NumericalAtom::math_divide(NumericalAtom* other) {
        value.value = value.value->math_div(other->value.value.get());
        if (value.units && other->value.units) {
	  value.units = (*value.units) / (*other->value.units);	  
        } else if (other->value.units) {
	  value.units = 1. / (*other->value.units);
        }	
    }
    void NumericalAtom::math_add(NumericalAtom* other) {
        if (value.units && other->value.units) {
            puq::Quantity quantity = std::move(other->value.value) * (*other->value.units);
            quantity = quantity.convert(*value.units);
            val::BaseValue::PointerType new_value = std::move(quantity.measurement.result.estimate);
            value.value = value.value->math_add(new_value.get());
        } else if (value.units) {
            throw std::runtime_error("NumericalAtom: Trying add nondimensional quantity to '" +
                                     other->value.units->to_string() + "'");
        } else if (other->value.units) {
            throw std::runtime_error("NumericalAtom: Trying add '" + value.units->to_string() +
                                     "' to a nondimensional quantity");
        } else {
            value.value = value.value->math_add(other->value.value.get());
        }
    }
    void NumericalAtom::math_subtract(NumericalAtom* other) {
        if (value.units && other->value.units) {
            puq::Quantity quantity = std::move(other->value.value) * (*other->value.units);
            quantity = quantity.convert(*value.units);
            val::BaseValue::PointerType new_value = std::move(quantity.measurement.result.estimate);
            value.value = value.value->math_sub(new_value.get());
        } else if (value.units) {
            throw std::runtime_error("NumericalAtom: Trying to subtract nondimensional quantity from '" +
                                     other->value.units->to_string() + "'");
        } else if (other->value.units) {
            throw std::runtime_error("NumericalAtom: Trying to subtract '" + value.units->to_string() +
                                     "' from a nondimensional quantity");
        } else {
            value.value = value.value->math_sub(other->value.value.get());
        }
    }
    void NumericalAtom::math_negate() {
        value.value = value.value->math_neg();
    }

} // namespace snt::dip
