#include "solver_logical.h"
#include "../nodes/node_boolean.h"
#include "../nodes/node_real.h"
#include "../nodes/node_integer.h"
#include "../nodes/node_string.h"

namespace snt::dip {

  LogicalAtom::LogicalAtom(const LogicalAtom& a) : AtomBase(a.value->clone()) {}

  LogicalAtom& LogicalAtom::operator=(const LogicalAtom& a) {
    if (this != &a)
      value = a.value->clone();
    return *this;
  }

  val::BaseValue::PointerType LogicalAtom::from_string(const std::string& s, LogicalSettings* settings) {
    Parser parser({s, {"LOGICAL_ATOM", 0}});
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
        node = RealNode::is_node(parser);
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

  std::string LogicalAtom::to_string() {
    return value->to_string();
  }

  // Comparison operations
  void LogicalAtom::comparison_equal(LogicalAtom* other) {
    value = value->compare_equal(other->value.get());
  }
  void LogicalAtom::comparison_not_equal(LogicalAtom* other) {
    value = value->compare_not_equal(other->value.get());
  }
  void LogicalAtom::comparison_less_equal(LogicalAtom* other) {
    value = value->compare_less_equal(other->value.get());
  }
  void LogicalAtom::comparison_greater_equal(LogicalAtom* other) {
    value = value->compare_greater_equal(other->value.get());
  }
  void LogicalAtom::comparison_less(LogicalAtom* other) {
    value = value->compare_less(other->value.get());
  }
  void LogicalAtom::comparison_greater(LogicalAtom* other) {
    value = value->compare_greater(other->value.get());
  }

  // Logical operations
  void LogicalAtom::logical_not() {
    value = value->logical_not();
  }
  void LogicalAtom::logical_and(LogicalAtom* other) {
    value = value->logical_and(other->value.get());
  }
  void LogicalAtom::logical_or(LogicalAtom* other) {
    value = value->logical_or(other->value.get());
  }

  LogicalSolver::LogicalSolver(Environment& env) {

    LogicalSettings settings = {&env};

    exs::OperatorList<LogicalAtom, LogicalSettings> operators;
    operators.append(
        exs::PARENTHESES_OPERATOR,
        std::make_shared<exs::OperatorParentheses<LogicalAtom, LogicalSettings>>(exs::OperatorGroupSybols("", "( ", " )", ", ")));
    operators.append(exs::EQUAL_OPERATOR,
                     std::make_shared<exs::OperatorEqual<LogicalAtom, LogicalSettings>>(" == "));
    operators.append(exs::NOT_EQUAL_OPERATOR,
                     std::make_shared<exs::OperatorNotEqual<LogicalAtom, LogicalSettings>>(" != "));
    operators.append(
        exs::NOT_OPERATOR,
        std::make_shared<exs::OperatorNot<LogicalAtom, LogicalSettings>>()); // needs to be after
                                                                             // NOT_EQUAL
    operators.append(
        exs::LESS_EQUAL_OPERATOR,
        std::make_shared<exs::OperatorLessEqual<LogicalAtom, LogicalSettings>>(" <= "));
    operators.append(
        exs::GREATER_EQUAL_OPERATOR,
        std::make_shared<exs::OperatorGreaterEqual<LogicalAtom, LogicalSettings>>(" >= "));
    operators.append(exs::LESS_OPERATOR,
                     std::make_shared<exs::OperatorLess<LogicalAtom, LogicalSettings>>(" < "));
    operators.append(exs::GREATER_OPERATOR,
                     std::make_shared<exs::OperatorGreater<LogicalAtom, LogicalSettings>>(" > "));
    operators.append(exs::AND_OPERATOR,
                     std::make_shared<exs::OperatorAnd<LogicalAtom, LogicalSettings>>(" && "));
    operators.append(exs::OR_OPERATOR,
                     std::make_shared<exs::OperatorOr<LogicalAtom, LogicalSettings>>(" || "));

    exs::StepList steps;
    steps.append(exs::GROUP_OPERATION, {exs::PARENTHESES_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::LESS_EQUAL_OPERATOR, exs::GREATER_EQUAL_OPERATOR,
                                         exs::LESS_OPERATOR, exs::GREATER_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::EQUAL_OPERATOR, exs::NOT_EQUAL_OPERATOR});
    steps.append(exs::UNARY_OPERATION, {exs::NOT_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::AND_OPERATOR});
    steps.append(exs::BINARY_OPERATION, {exs::OR_OPERATOR});

    solver =
        std::make_unique<exs::Solver<LogicalAtom, LogicalSettings>>(operators, steps, settings);
  }

  LogicalAtom LogicalSolver::eval(const std::string& expression) {
    if (expression.empty())
      throw std::runtime_error("Logical expression cannot be empty");
    LogicalAtom ua = solver->solve(expression);
    return ua;
  }

} // namespace snt::dip
