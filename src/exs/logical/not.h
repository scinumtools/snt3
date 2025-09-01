#ifndef EXS_OPERATOR_NOT_H
#define EXS_OPERATOR_NOT_H

namespace snt::exs {

  template <class A, typename S = EmptySettings>
  class OperatorNot : public OperatorBase<A, S> {
  public:
    OperatorNot(std::string s = "!") : OperatorBase<A, S>("not", s, NOT_OPERATOR) {}
    void operate_unary(TokenListBase<A>* tokens) override {
      Token<A> right = tokens->get_right();
      right.atom->logical_not();
      tokens->put_right(right);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_NOT_H
