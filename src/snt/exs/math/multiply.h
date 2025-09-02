#ifndef EXS_OPERATOR_MULTIPLY_H
#define EXS_OPERATOR_MULTIPLY_H

namespace snt::exs {

  template <class A, typename S = EmptySettings>
  class OperatorMultiply : public OperatorBase<A, S> {
  public:
    OperatorMultiply(std::string s = "*") : OperatorBase<A, S>("mul", s, MULTIPLY_OPERATOR) {}
    void operate_binary(TokenListBase<A>* tokens) override {
      Token<A> left = tokens->get_left();
      Token<A> right = tokens->get_right();
      left.atom->math_multiply(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_MULTIPLY_H
