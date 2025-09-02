#ifndef EXS_OPERATOR_GREATER_H
#define EXS_OPERATOR_GREATER_H

namespace snt::exs {

  template <class A, typename S = EmptySettings>
  class OperatorGreater : public OperatorBase<A, S> {
  public:
    OperatorGreater(std::string s = ">") : OperatorBase<A, S>("gt", s, GREATER_OPERATOR) {}
    void operate_binary(TokenListBase<A>* tokens) override {
      Token<A> left = tokens->get_left();
      Token<A> right = tokens->get_right();
      left.atom->comparison_greater(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_GREATER_H
