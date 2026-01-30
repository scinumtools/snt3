#ifndef EXS_OPERATOR_MULTIPLY_H
#define EXS_OPERATOR_MULTIPLY_H

namespace snt::exs {

  class OperatorMultiply : public OperatorBase {
  public:
    OperatorMultiply(std::string s = "*") : OperatorBase("mul", s, MULTIPLY_OPERATOR) {}
    void operate_binary(TokenListBase* tokens) override {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->math_multiply(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_MULTIPLY_H
