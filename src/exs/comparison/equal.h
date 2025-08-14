#ifndef EXS_OPERATOR_EQUAL_H
#define EXS_OPERATOR_EQUAL_H

namespace exs {

  template <class A, typename S = EmptySettings>
  class OperatorEqual : public OperatorBase<A, S> {
  public:
    OperatorEqual(std::string s = "==") : OperatorBase<A, S>("eq", s, EQUAL_OPERATOR) {}
    void operate_binary(TokenListBase<A>* tokens) override {
      Token<A> left = tokens->get_left();
      Token<A> right = tokens->get_right();
      left.atom->comparison_equal(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace exs

#endif // EXS_OPERATOR_EQUAL_H
