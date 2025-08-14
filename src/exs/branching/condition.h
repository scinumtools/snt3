#ifndef EXS_OPERATOR_CONDITION_H
#define EXS_OPERATOR_CONDITION_H

namespace exs {

  template <class A, typename S = EmptySettings>
  class OperatorCondition : public OperatorTernary<A, S> {
  public:
    OperatorCondition() : OperatorTernary<A, S>("cond", "?", ":", CONDITION_OPERATOR) {}
    void operate_ternary(TokenListBase<A>* tokens) override {
      Token<A> group2 = tokens->get_left();
      Token<A> group1 = tokens->get_left();
      Token<A> group3 = tokens->get_right();
      group1.atom->condition(group2.atom, group3.atom);
      tokens->put_left(group1);
    };
  };

} // namespace exs

#endif // EXS_OPERATOR_CONDITION_H
