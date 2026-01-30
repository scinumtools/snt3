#ifndef EXS_OPERATOR_LESS_H
#define EXS_OPERATOR_LESS_H

namespace snt::exs {

  class OperatorLess : public OperatorBase {
  public:
    OperatorLess(std::string s = "<") : OperatorBase("lt", s, LESS_OPERATOR) {}
    void operate_binary(TokenListBase* tokens) override {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->comparison_less(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_LESS_H
