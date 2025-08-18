#ifndef EXS_OPERATOR_CUBIC_ROOT_H
#define EXS_OPERATOR_CUBIC_ROOT_H

namespace exs {

  template <class A, typename S = EmptySettings>
  class OperatorCubicRoot : public OperatorGroup<A, 1, S> {
  public:
    OperatorCubicRoot(const OperatorGroupSybols& s = {"cbrt", "(", ")", ","}) : OperatorGroup<A, 1, S>("cbrt", s, CUBIC_ROOT_OPERATOR) {}
    void operate_group(TokenListBase<A>* tokens) override {
      Token<A> group1 = tokens->get_left();
      group1.atom->math_cubic_root();
      tokens->put_left(group1);
    };
  };

} // namespace exs

#endif // EXS_OPERATOR_CUBIC_ROOT_H
