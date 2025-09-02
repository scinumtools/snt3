#ifndef EXS_OPERATOR_NOT_EQUAL_H
#define EXS_OPERATOR_NOT_EQUAL_H

namespace snt::exs {

  template <class A, typename S = EmptySettings>
  class OperatorNotEqual : public OperatorBase<A, S> {
  public:
    OperatorNotEqual(std::string s = "!=") : OperatorBase<A, S>("ne", s, NOT_EQUAL_OPERATOR) {}
    void operate_binary(TokenListBase<A>* tokens) override {
      Token<A> left = tokens->get_left();
      Token<A> right = tokens->get_right();
      left.atom->comparison_not_equal(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_NOT_EQUAL_H
