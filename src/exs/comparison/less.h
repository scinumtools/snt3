#ifndef EXS_OPERATOR_LESS_H
#define EXS_OPERATOR_LESS_H

namespace exs {

  template <class A, typename S = EmptySettings>
  class OperatorLess : public OperatorBase<A, S> {
  public:
    OperatorLess(std::string s = "<") : OperatorBase<A, S>("lt", s, LESS_OPERATOR) {}
    void operate_binary(TokenListBase<A>* tokens) override {
      Token<A> left = tokens->get_left();
      Token<A> right = tokens->get_right();
      left.atom->comparison_less(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace exs

#endif // EXS_OPERATOR_LESS_H
