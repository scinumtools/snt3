#ifndef EXS_OPERATOR_DIVIDE_H
#define EXS_OPERATOR_DIVIDE_H

namespace snt::exs {

  class OperatorDivide : public OperatorBase {
  public:
    OperatorDivide(std::string s = "/") : OperatorBase("div", s, DIVIDE_OPERATOR) {}
    void operate_binary(TokenListBase* tokens) override {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->math_divide(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_DIVIDE_H
