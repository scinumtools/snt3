#ifndef EXS_OPERATOR_LOGARITHM_H
#define EXS_OPERATOR_LOGARITHM_H

namespace snt::exs {

  class OperatorLogarithm : public OperatorGroup<1> {
  public:
    OperatorLogarithm(const OperatorGroupSybols& s = {"log", "(", ")", ","}) : OperatorGroup<1>("log", s, LOGARITHM_OPERATOR) {}
    void operate_group(TokenListBase* tokens) override {
      Token group1 = tokens->get_left();
      group1.atom->math_logarithm();
      tokens->put_left(group1);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_LOGARITHM_H
