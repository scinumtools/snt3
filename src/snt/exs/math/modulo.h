#ifndef EXS_OPERATOR_MODULO_H
#define EXS_OPERATOR_MODULO_H

namespace snt::exs {

  class OperatorModulo : public OperatorBase {
  public:
    OperatorModulo(std::string s = "%") : OperatorBase("mod", s, MODULO_OPERATOR) {}
    void operate_binary(TokenListBase* tokens) override {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->math_modulo(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_MODULO_H
