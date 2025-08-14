#ifndef EXS_OPERATOR_AND_H
#define EXS_OPERATOR_AND_H

namespace exs {

  template <class A, typename S = EmptySettings>
  class OperatorAnd : public OperatorBase<A, S> {
  public:
    OperatorAnd(std::string s = "&&") : OperatorBase<A, S>("and", s, AND_OPERATOR) {}
    void operate_binary(TokenListBase<A>* tokens) override {
      Token<A> left = tokens->get_left();
      Token<A> right = tokens->get_right();
      left.atom->logical_and(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace exs

#endif // EXS_OPERATOR_AND_H
