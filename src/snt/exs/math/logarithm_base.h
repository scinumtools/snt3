#ifndef EXS_OPERATOR_LOGARITHM_BASE_H
#define EXS_OPERATOR_LOGARITHM_BASE_H

namespace snt::exs {

  template <typename S = EmptySettings>
  class OperatorLogarithmBase : public OperatorGroup<2, S> {
  public:
    OperatorLogarithmBase(const OperatorGroupSybols& s = {"logb", "(", ")", ","}) : OperatorGroup<2, S>("logb", s, LOGARITHM_BASE_OPERATOR) {}
    void operate_group(TokenListBase* tokens) override {
      Token group2 = tokens->get_left();
      Token group1 = tokens->get_left();
      group1.atom->math_logarithm_base(group2.atom);
      tokens->put_left(group1);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_LOGARITHM_BASE_H
