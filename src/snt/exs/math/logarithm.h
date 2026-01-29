#ifndef EXS_OPERATOR_LOGARITHM_H
#define EXS_OPERATOR_LOGARITHM_H

namespace snt::exs {

  template <typename S = EmptySettings>
  class OperatorLogarithm : public OperatorGroup<1, S> {
  public:
    OperatorLogarithm(const OperatorGroupSybols& s = {"log", "(", ")", ","}) : OperatorGroup<1, S>("log", s, LOGARITHM_OPERATOR) {}
    void operate_group(TokenListBase* tokens) override {
      Token group1 = tokens->get_left();
      group1.atom->math_logarithm();
      tokens->put_left(group1);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_LOGARITHM_H
