#ifndef EXS_OPERATOR_NOT_H
#define EXS_OPERATOR_NOT_H

namespace snt::exs {

  class OperatorNot : public OperatorBase {
  public:
    OperatorNot(std::string s = "!") : OperatorBase("not", s, NOT_OPERATOR) {}
    void operate_unary(TokenListBase* tokens) override {
      Token right = tokens->get_right();
      right.atom->logical_not();
      tokens->put_right(right);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_NOT_H
