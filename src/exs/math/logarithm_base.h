#ifndef EXS_OPERATOR_LOGARITHM_BASE_H
#define EXS_OPERATOR_LOGARITHM_BASE_H

namespace exs {

  template <class A, typename S = EmptySettings>
  class OperatorLogarithmBase : public OperatorGroup<A, 2, S> {
  public:
    OperatorLogarithmBase(const OperatorGroupSybols& s = {"logb", "(", ")", ","}) : OperatorGroup<A, 2, S>("logb", s, LOGARITHM_BASE_OPERATOR) {}
    void operate_group(TokenListBase<A>* tokens) override {
      Token<A> group2 = tokens->get_left();
      Token<A> group1 = tokens->get_left();
      group1.atom->math_logarithm_base(group2.atom);
      tokens->put_left(group1);
    };
  };

} // namespace exs

#endif // EXS_OPERATOR_LOGARITHM_BASE_H
