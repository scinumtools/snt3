#ifndef EXS_OPERATOR_CUBIC_ROOT_H
#define EXS_OPERATOR_CUBIC_ROOT_H

namespace snt::exs {

  class OperatorCubicRoot : public OperatorGroup<1> {
  public:
    OperatorCubicRoot(const OperatorGroupSybols& s = {"cbrt", "(", ")", ","}) : OperatorGroup<1>("cbrt", s, CUBIC_ROOT_OPERATOR) {}
    void operate_group(TokenListBase* tokens) override {
      Token group1 = tokens->get_left();
      group1.atom->math_cubic_root();
      tokens->put_left(group1);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_CUBIC_ROOT_H
