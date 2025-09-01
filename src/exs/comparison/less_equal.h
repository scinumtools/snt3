#ifndef EXS_OPERATOR_LESS_EQUAL_H
#define EXS_OPERATOR_LESS_EQUAL_H

namespace snt::exs {

  template <class A, typename S = EmptySettings>
  class OperatorLessEqual : public OperatorBase<A, S> {
  public:
    OperatorLessEqual(std::string s = "<=") : OperatorBase<A, S>("le", s, LESS_EQUAL_OPERATOR) {}
    void operate_binary(TokenListBase<A>* tokens) override {
      Token<A> left = tokens->get_left();
      Token<A> right = tokens->get_right();
      left.atom->comparison_less_equal(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_LESS_EQUAL_H
