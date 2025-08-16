#include "../nodes/nodes.h"
#include "solvers.h"

namespace dip {

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
      //if (node == nullptr)
      //  node = StringNode::is_node(parser);
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

  //// Comparison operations
  //void NumericalAtom::comparison_equal(NumericalAtom* other) {
  //  value = value->compare_equal(other->value.get());
  //}
  //void NumericalAtom::comparison_not_equal(NumericalAtom* other) {
  //  value = value->compare_not_equal(other->value.get());
  //}
  //void NumericalAtom::comparison_less_equal(NumericalAtom* other) {
  //  value = value->compare_less_equal(other->value.get());
  //}
  //void NumericalAtom::comparison_greater_equal(NumericalAtom* other) {
  //  value = value->compare_greater_equal(other->value.get());
  //}
  //void NumericalAtom::comparison_less(NumericalAtom* other) {
  //  value = value->compare_less(other->value.get());
  //}
  //void NumericalAtom::comparison_greater(NumericalAtom* other) {
  //  value = value->compare_greater(other->value.get());
  //}
  //
  //// Numerical operations
  //void NumericalAtom::numerical_not() {
  //  value = value->numerical_not();
  //}
  //void NumericalAtom::numerical_and(NumericalAtom* other) {
  //  value = value->numerical_and(other->value.get());
  //}
  //void NumericalAtom::numerical_or(NumericalAtom* other) {
  //  value = value->numerical_or(other->value.get());
  //}

  NumericalSolver::NumericalSolver(Environment& env) {

    NumericalSettings settings = {&env};

    exs::OperatorList<NumericalAtom, NumericalSettings> operators;
    operators.append(
        exs::PARENTHESES_OPERATOR,
        std::make_shared<exs::OperatorParentheses<NumericalAtom, NumericalSettings>>("( ", " )"));
    //operators.append(exs::EQUAL_OPERATOR,
    //                 std::make_shared<exs::OperatorEqual<NumericalAtom, NumericalSettings>>(" == "));
    //operators.append(exs::NOT_EQUAL_OPERATOR,
    //                 std::make_shared<exs::OperatorNotEqual<NumericalAtom, NumericalSettings>>(" != "));
    //operators.append(
    //    exs::NOT_OPERATOR,
    //    std::make_shared<exs::OperatorNot<NumericalAtom, NumericalSettings>>()); // needs to be after
    //                                                                         // NOT_EQUAL
    //operators.append(
    //    exs::LESS_EQUAL_OPERATOR,
    //    std::make_shared<exs::OperatorLessEqual<NumericalAtom, NumericalSettings>>(" <= "));
    //operators.append(
    //    exs::GREATER_EQUAL_OPERATOR,
    //    std::make_shared<exs::OperatorGreaterEqual<NumericalAtom, NumericalSettings>>(" >= "));
    //operators.append(exs::LESS_OPERATOR,
    //                 std::make_shared<exs::OperatorLess<NumericalAtom, NumericalSettings>>(" < "));
    //operators.append(exs::GREATER_OPERATOR,
    //                 std::make_shared<exs::OperatorGreater<NumericalAtom, NumericalSettings>>(" > "));
    //operators.append(exs::AND_OPERATOR,
    //                 std::make_shared<exs::OperatorAnd<NumericalAtom, NumericalSettings>>(" && "));
    //operators.append(exs::OR_OPERATOR,
    //                 std::make_shared<exs::OperatorOr<NumericalAtom, NumericalSettings>>(" || "));

    exs::StepList steps;
    steps.append(exs::GROUP_OPERATION, {exs::PARENTHESES_OPERATOR});
    //steps.append(exs::BINARY_OPERATION, {exs::LESS_EQUAL_OPERATOR, exs::GREATER_EQUAL_OPERATOR,
    //                                     exs::LESS_OPERATOR, exs::GREATER_OPERATOR});
    //steps.append(exs::BINARY_OPERATION, {exs::EQUAL_OPERATOR, exs::NOT_EQUAL_OPERATOR});
    //steps.append(exs::UNARY_OPERATION, {exs::NOT_OPERATOR});
    //steps.append(exs::BINARY_OPERATION, {exs::AND_OPERATOR});
    //steps.append(exs::BINARY_OPERATION, {exs::OR_OPERATOR});

    solver =
        std::make_unique<exs::Solver<NumericalAtom, NumericalSettings>>(operators, steps, settings);
  }

  NumericalAtom NumericalSolver::eval(const std::string& expression) {
    if (expression.empty())
      throw std::runtime_error("Numerical expression cannot be empty");
    NumericalAtom ua = solver->solve(expression);
    return ua;
  }
  
}


