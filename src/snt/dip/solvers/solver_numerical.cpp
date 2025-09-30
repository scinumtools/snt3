#include "solver_numerical.h"

#include "../nodes/node_boolean.h"
#include "../nodes/node_float.h"
#include "../nodes/node_integer.h"
#include "../nodes/node_string.h"

namespace snt::dip {

  NumericalAtom::NumericalAtom(const NumericalAtom& a) : AtomBase(a.value->clone()) {}

  NumericalAtom& NumericalAtom::operator=(const NumericalAtom& a) {
    if (this != &a)
      value = a.value->clone();
    return *this;
  }

  val::BaseValue::PointerType NumericalAtom::from_string(const std::string& s, NumericalSettings* settings) {
    Parser parser({s, {"NUMERICAL_ATOM", 0}});
    if (parser.part_reference()) {
      val::BaseValue::PointerType value =
          settings->env->request_value(parser.value_raw.at(0), RequestType::Reference);
      return std::move(value);
    } else if (parser.part_literal()) {
      BaseNode::PointerType node = nullptr;
      if (node == nullptr)
        node = BooleanNode::is_node(parser);
      if (node == nullptr)
        node = IntegerNode::is_node(parser);
      if (node == nullptr)
        node = FloatNode::is_node(parser);
      if (node == nullptr)
        node = StringNode::is_node(parser);
      if (node == nullptr)
        throw std::runtime_error("Value could not be determined from : " + s);
      ValueNode::PointerType vnode = std::dynamic_pointer_cast<ValueNode>(node);
      vnode->set_value();
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

  NumericalSolver::NumericalSolver(Environment& env) {

    NumericalSettings settings = {&env};

    exs::OperatorList<NumericalAtom, NumericalSettings> operators;
    operators.append(
        exs::SINUS_OPERATOR,
        std::make_shared<exs::OperatorSinus<NumericalAtom, NumericalSettings>>(exs::OperatorGroupSybols("sin", "( ", " )", ", ")));
    operators.append(
        exs::COSINUS_OPERATOR,
        std::make_shared<exs::OperatorCosinus<NumericalAtom, NumericalSettings>>(exs::OperatorGroupSybols("cos", "( ", " )", ", ")));
    operators.append(
        exs::TANGENS_OPERATOR,
        std::make_shared<exs::OperatorTangens<NumericalAtom, NumericalSettings>>(exs::OperatorGroupSybols("tan", "( ", " )", ", ")));
    operators.append(
        exs::CUBIC_ROOT_OPERATOR,
        std::make_shared<exs::OperatorCubicRoot<NumericalAtom, NumericalSettings>>(exs::OperatorGroupSybols("cbrt", "( ", " )", ", ")));
    operators.append(
        exs::SQUARE_ROOT_OPERATOR,
        std::make_shared<exs::OperatorSquareRoot<NumericalAtom, NumericalSettings>>(exs::OperatorGroupSybols("sqrt", "( ", " )", ", ")));
    // operators.append(
    //     exs::POWER_BASE_OPERATOR,
    //     std::make_shared<exs::OperatorPowerBase<NumericalAtom, NumericalSettings>>("powb( ", " )"));
    // operators.append(
    //     exs::LOGARITHM_BASE_OPERATOR,
    //     std::make_shared<exs::OperatorLogarithmBase<NumericalAtom, NumericalSettings>>("logb( ", " )"));
    operators.append(
        exs::LOGARITHM_10_OPERATOR,
        std::make_shared<exs::OperatorLogarithm10<NumericalAtom, NumericalSettings>>(exs::OperatorGroupSybols("log10", "( ", " )", ", ")));
    operators.append(
        exs::LOGARITHM_OPERATOR,
        std::make_shared<exs::OperatorLogarithm<NumericalAtom, NumericalSettings>>(exs::OperatorGroupSybols("log", "( ", " )", ", ")));
    operators.append(
        exs::EXPONENT_OPERATOR,
        std::make_shared<exs::OperatorExponent<NumericalAtom, NumericalSettings>>(exs::OperatorGroupSybols("exp", "( ", " )", ", ")));
    operators.append(
        exs::PARENTHESES_OPERATOR,
        std::make_shared<exs::OperatorParentheses<NumericalAtom, NumericalSettings>>(exs::OperatorGroupSybols("", "( ", " )", ", ")));
    operators.append(exs::POWER_OPERATOR,
                     std::make_shared<exs::OperatorPower<NumericalAtom, NumericalSettings>>(" ** "));
    operators.append(exs::MULTIPLY_OPERATOR,
                     std::make_shared<exs::OperatorMultiply<NumericalAtom, NumericalSettings>>(" * "));
    operators.append(exs::DIVIDE_OPERATOR,
                     std::make_shared<exs::OperatorDivide<NumericalAtom, NumericalSettings>>(" / "));
    operators.append(exs::ADD_OPERATOR,
                     std::make_shared<exs::OperatorAdd<NumericalAtom, NumericalSettings>>(" +"));
    operators.append(exs::SUBTRACT_OPERATOR,
                     std::make_shared<exs::OperatorSubtract<NumericalAtom, NumericalSettings>>(" -"));

    exs::StepList steps;
    steps.append(exs::GROUP_OPERATION, {exs::EXPONENT_OPERATOR, exs::LOGARITHM_OPERATOR,
                                        exs::LOGARITHM_10_OPERATOR,
                                        // exs::LOGARITHM_BASE_OPERATOR, exs::POWER_BASE_OPERATOR,
                                        exs::SQUARE_ROOT_OPERATOR,
                                        exs::CUBIC_ROOT_OPERATOR,
                                        exs::SINUS_OPERATOR, exs::COSINUS_OPERATOR, exs::TANGENS_OPERATOR});
    steps.append(exs::GROUP_OPERATION, {exs::PARENTHESES_OPERATOR});
    steps.append(exs::UNARY_OPERATION, {exs::ADD_OPERATOR, exs::SUBTRACT_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::POWER_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::MULTIPLY_OPERATOR, exs::DIVIDE_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::ADD_OPERATOR, exs::SUBTRACT_OPERATOR});

    solver =
        std::make_unique<exs::Solver<NumericalAtom, NumericalSettings>>(operators, steps, settings);
  }

  NumericalAtom NumericalSolver::eval(const std::string& expression) {
    if (expression.empty())
      throw std::runtime_error("Numerical expression cannot be empty");
    NumericalAtom ua = solver->solve(expression);
    return ua;
  }

} // namespace snt::dip
