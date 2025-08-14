#ifndef EXS_OPERATOR_LOGARITHM_H
#define EXS_OPERATOR_LOGARITHM_H

namespace exs {

  template <class A, typename S = EmptySettings>
  class OperatorLogarithm : public OperatorGroup<A, 1, S> {
  public:
    OperatorLogarithm() : OperatorGroup<A, 1, S>("log", "log(", LOGARITHM_OPERATOR) {}
    void operate_group(TokenListBase<A>* tokens) override {
      Token<A> group1 = tokens->get_left();
      group1.atom->math_logarithm();
      tokens->put_left(group1);
    };
  };

} // namespace exs

#endif // EXS_OPERATOR_LOGARITHM_H
