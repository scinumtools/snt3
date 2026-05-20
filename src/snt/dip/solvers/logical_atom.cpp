#include <snt/dip/nodes/node_boolean.h>
#include <snt/dip/nodes/node_float.h>
#include <snt/dip/nodes/node_integer.h>
#include <snt/dip/nodes/node_string.h>
#include <snt/dip/solvers/logical_atom.h>

namespace snt::dip {

  LogicalAtom::LogicalAtom(const LogicalAtom& a) : AtomBase(a.value->clone()) {}

  LogicalAtom& LogicalAtom::operator=(const LogicalAtom& a) {
    if (this != &a)
      value = a.value->clone();
    return *this;
  }

  val::BaseValue::PointerType LogicalAtom::from_string(const std::string& s, exs::BaseSettings* settings) {
    Parser parser({s, {"LOGICAL_ATOM", 0}});
    if (parser.part_reference()) {
      LogicalSettings* csettings = static_cast<LogicalSettings*>(settings);
      val::BaseValue::PointerType value =
          csettings->env->request_value(parser.value_raw.at(0), RequestType::Reference);
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

} // namespace snt::dip
