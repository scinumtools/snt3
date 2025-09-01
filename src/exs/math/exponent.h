#ifndef EXS_OPERATOR_EXPONENT_H
#define EXS_OPERATOR_EXPONENT_H

namespace snt::exs {

  template <class A, typename S = EmptySettings>
  class OperatorExponent : public OperatorGroup<A, 1, S> {
  public:
    OperatorExponent(const OperatorGroupSybols& s = {"exp", "(", ")", ","}) : OperatorGroup<A, 1, S>("exp", s, EXPONENT_OPERATOR) {}
    void operate_group(TokenListBase<A>* tokens) override {
      Token<A> group1 = tokens->get_left();
      group1.atom->math_exponent();
      tokens->put_left(group1);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_EXPONENT_H
