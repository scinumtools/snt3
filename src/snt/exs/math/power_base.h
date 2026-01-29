#ifndef EXS_OPERATOR_POWER_BASE_H
#define EXS_OPERATOR_POWER_BASE_H

namespace snt::exs {

  template <typename S = EmptySettings>
  class OperatorPowerBase : public OperatorGroup<2, S> {
  public:
    OperatorPowerBase(const OperatorGroupSybols& s = {"powb", "(", ")", ","}) : OperatorGroup<2, S>("powb", s, POWER_BASE_OPERATOR) {}
    void operate_group(TokenListBase* tokens) override {
      Token group2 = tokens->get_left();
      Token group1 = tokens->get_left();
      group1.atom->math_power_base(group2.atom);
      tokens->put_left(group1);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_POWER_BASE_H
