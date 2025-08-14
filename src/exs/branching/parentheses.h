#ifndef EXS_OPERATOR_PARENTHESES_H
#define EXS_OPERATOR_PARENTHESES_H

namespace exs {

  template <class A, typename S = EmptySettings>
  class OperatorParentheses : public OperatorGroup<A, 1, S> {
  public:
    OperatorParentheses(const std::string so = "(", const std::string sc = ")") : OperatorGroup<A, 1, S>("par", so, PARENTHESES_OPERATOR, so, sc, ",") {}
  };

} // namespace exs

#endif // EXS_OPERATOR_PARENTHESES_H
