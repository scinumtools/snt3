#ifndef EXS_OPERATOR_POWER_BASE_H
#define EXS_OPERATOR_POWER_BASE_H

namespace exs {

  template <class A, typename S = EmptySettings>
  class OperatorPowerBase : public OperatorGroup<A, 2, S> {
  public:
    OperatorPowerBase(const OperatorGroupSybols& s = {"powb", "(", ")", ","}) : OperatorGroup<A, 2, S>("powb", s, POWER_BASE_OPERATOR) {}
    void operate_group(TokenListBase<A>* tokens) override {
      Token<A> group2 = tokens->get_left();
      Token<A> group1 = tokens->get_left();
      group1.atom->math_power_base(group2.atom);
      tokens->put_left(group1);
    };
  };

} // namespace exs

#endif // EXS_OPERATOR_POWER_BASE_H
