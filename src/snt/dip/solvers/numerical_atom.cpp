#include <snt/dip/nodes/node_boolean.h>
#include <snt/dip/nodes/node_float.h>
#include <snt/dip/nodes/node_integer.h>
#include <snt/dip/nodes/node_string.h>
#include <snt/dip/solvers/numerical_atom.h>

namespace snt::dip {

  NumericalAtom::NumericalAtom(const NumericalAtom& a) : AtomBase(a.value->clone()) {}

  NumericalAtom& NumericalAtom::operator=(const NumericalAtom& a) {
    if (this != &a)
      value = a.value->clone();
    return *this;
  }

  val::BaseValue::PointerType NumericalAtom::from_string(const std::string& s, exs::BaseSettings* settings) {
    Parser parser({s, {"NUMERICAL_ATOM", 0}});
    NumericalSettings* csettings = static_cast<NumericalSettings*>(settings);
    if (parser.part_reference()) {
      val::BaseValue::PointerType value =
          csettings->env->request_value(parser.value_raw.at(0), RequestType::Reference, csettings->units);
      return std::move(value);
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
      if (!vnode->units_raw.empty() && !csettings->units.empty()) {
        puq::Quantity quantity(std::move(vnode->value), vnode->units_raw);
        quantity = quantity.convert(csettings->units);
        vnode->value = std::move(quantity.measurement.result.estimate);
      } else if (vnode->units_raw.empty() && !csettings->units.empty())
        throw std::runtime_error(
            "Numerical Solver: Trying to convert nondimensional quantity into '" + csettings->units +
            "': " + vnode->line.code);
      else if (!vnode->units_raw.empty() && csettings->units.empty())
        throw std::runtime_error(
            "Numerical Solver: Trying to convert '" + vnode->units_raw +
            "' into a nondimensional quantity: " + vnode->line.code);
      return std::move(vnode->value);
    } else {
      throw std::runtime_error("Invalid atom value: " + s);
    }
  }

  std::string NumericalAtom::to_string() {
    return value->to_string();
  }

  // Mathematical operations
  void NumericalAtom::math_sinus() {
    value = value->math_sin();
  }
  void NumericalAtom::math_cosinus() {
    value = value->math_cos();
  }
  void NumericalAtom::math_tangens() {
    value = value->math_tan();
  }
  void NumericalAtom::math_cubic_root() {
    value = value->math_cbrt();
  }
  void NumericalAtom::math_square_root() {
    value = value->math_sqrt();
  }
  // void NumericalAtom::math_power_base(NumericalAtom* other) {
  //   value = value->math_powb(other->value.get());
  // }
  // void NumericalAtom::math_logarithm_base(NumericalAtom* other) {
  //   value = value->math_logb(other->value.get());
  // }
  void NumericalAtom::math_logarithm_10() {
    value = value->math_log10();
  }
  void NumericalAtom::math_logarithm() {
    value = value->math_log();
  }
  void NumericalAtom::math_exponent() {
    value = value->math_exp();
  }
  void NumericalAtom::math_power(NumericalAtom* other) {
    value = value->math_pow(other->value.get());
  }
  void NumericalAtom::math_multiply(NumericalAtom* other) {
    value = value->math_mul(other->value.get());
  }
  void NumericalAtom::math_divide(NumericalAtom* other) {
    value = value->math_div(other->value.get());
  }
  void NumericalAtom::math_add(NumericalAtom* other) {
    value = value->math_add(other->value.get());
  }
  void NumericalAtom::math_subtract(NumericalAtom* other) {
    value = value->math_sub(other->value.get());
  }
  void NumericalAtom::math_negate() {
    value = value->math_neg();
  }

} // namespace snt::dip
