#ifndef EXS_OPERATOR_OR_H
#define EXS_OPERATOR_OR_H

namespace exs {

  template <class A, typename S = EmptySettings>
  class OperatorOr : public OperatorBase<A, S> {
  public:
    OperatorOr(std::string s = "||") : OperatorBase<A, S>("or", s, OR_OPERATOR) {}
    void operate_binary(TokenListBase<A>* tokens) override {
      Token<A> left = tokens->get_left();
      Token<A> right = tokens->get_right();
      left.atom->logical_or(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace exs

#endif // EXS_OPERATOR_OR_H
