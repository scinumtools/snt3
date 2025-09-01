#ifndef EXS_OPERATOR_LOGARITHM_10_H
#define EXS_OPERATOR_LOGARITHM_10_H

namespace snt::exs {

  template <class A, typename S = EmptySettings>
  class OperatorLogarithm10 : public OperatorGroup<A, 1, S> {
  public:
    OperatorLogarithm10(const OperatorGroupSybols& s = {"log10", "(", ")", ","}) : OperatorGroup<A, 1, S>("log10", s, LOGARITHM_10_OPERATOR) {}
    void operate_group(TokenListBase<A>* tokens) override {
      Token<A> group1 = tokens->get_left();
      group1.atom->math_logarithm_10();
      tokens->put_left(group1);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_LOGARITHM_10_H
