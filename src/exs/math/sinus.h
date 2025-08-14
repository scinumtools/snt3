#ifndef EXS_OPERATOR_SINUS_H
#define EXS_OPERATOR_SINUS_H

namespace exs {

  template <class A, typename S = EmptySettings>
  class OperatorSinus : public OperatorGroup<A, 1, S> {
  public:
    OperatorSinus() : OperatorGroup<A, 1, S>("sin", "sin(", SINUS_OPERATOR) {}
    void operate_group(TokenListBase<A>* tokens) override {
      Token<A> group1 = tokens->get_left();
      group1.atom->math_sinus();
      tokens->put_left(group1);
    };
  };

} // namespace exs

#endif // EXS_OPERATOR_SINUS_H
