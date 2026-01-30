#ifndef EXS_OPERATOR_EXPONENT_H
#define EXS_OPERATOR_EXPONENT_H

namespace snt::exs {

  class OperatorExponent : public OperatorGroup<1> {
  public:
    OperatorExponent(const OperatorGroupSybols& s = {"exp", "(", ")", ","}) : OperatorGroup<1>("exp", s, EXPONENT_OPERATOR) {}
    void operate_group(TokenListBase* tokens) override {
      Token group1 = tokens->get_left();
      group1.atom->math_exponent();
      tokens->put_left(group1);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_EXPONENT_H
