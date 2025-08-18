#ifndef EXS_OPERATOR_COSINUS_H
#define EXS_OPERATOR_COSINUS_H

namespace exs {

  template <class A, typename S = EmptySettings>
  class OperatorCosinus : public OperatorGroup<A, 1, S> {
  public:
    OperatorCosinus(const OperatorGroupSybols& s = {"cos", "(", ")", ","}) : OperatorGroup<A, 1, S>("cos", s, COSINUS_OPERATOR) {}
    void operate_group(TokenListBase<A>* tokens) override {
      Token<A> group1 = tokens->get_left();
      group1.atom->math_cosinus();
      tokens->put_left(group1);
    };
  };

} // namespace exs

#endif // EXS_OPERATOR_COSINUS_H
