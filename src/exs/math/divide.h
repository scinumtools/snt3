#ifndef EXS_OPERATOR_DIVIDE_H
#define EXS_OPERATOR_DIVIDE_H

namespace exs {

  template <class A, typename S = EmptySettings>
  class OperatorDivide : public OperatorBase<A, S> {
  public:
    OperatorDivide(std::string s = "/") : OperatorBase<A, S>("div", s, DIVIDE_OPERATOR) {}
    void operate_binary(TokenListBase<A>* tokens) override {
      Token<A> left = tokens->get_left();
      Token<A> right = tokens->get_right();
      left.atom->math_divide(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace exs

#endif // EXS_OPERATOR_DIVIDE_H
