#ifndef EXS_OPERATOR_GREATER_EQUAL_H
#define EXS_OPERATOR_GREATER_EQUAL_H

namespace snt::exs {

  template <class A, typename S = EmptySettings>
  class OperatorGreaterEqual : public OperatorBase<A, S> {
  public:
    OperatorGreaterEqual(std::string s = ">=") : OperatorBase<A, S>("ge", s, GREATER_EQUAL_OPERATOR) {}
    void operate_binary(TokenListBase<A>* tokens) override {
      Token<A> left = tokens->get_left();
      Token<A> right = tokens->get_right();
      left.atom->comparison_greater_equal(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_GREATER_EQUAL_H
