#ifndef EXS_OPERATOR_OR_H
#define EXS_OPERATOR_OR_H

namespace snt::exs {

  class OperatorOr : public OperatorBase {
  public:
    OperatorOr(std::string s = "||") : OperatorBase("or", s, OR_OPERATOR) {}
    void operate_binary(TokenListBase* tokens) override {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->logical_or(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_OR_H
