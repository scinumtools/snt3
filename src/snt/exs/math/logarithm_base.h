#ifndef EXS_OPERATOR_LOGARITHM_BASE_H
#define EXS_OPERATOR_LOGARITHM_BASE_H

namespace snt::exs {

  class OperatorLogarithmBase : public OperatorGroup<2> {
  public:
    OperatorLogarithmBase(const OperatorGroupSybols& s = {"logb", "(", ")", ","}) : OperatorGroup<2>("logb", s, LOGARITHM_BASE_OPERATOR) {}
    void operate_group(TokenListBase* tokens) override {
      Token group2 = tokens->get_left();
      Token group1 = tokens->get_left();
      group1.atom->math_logarithm_base(group2.atom);
      tokens->put_left(group1);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_LOGARITHM_BASE_H
