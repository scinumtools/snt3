#ifndef EXS_OPERATOR_GREATER_H
#define EXS_OPERATOR_GREATER_H

namespace snt::exs {

  class OperatorGreater : public OperatorBase {
  public:
    OperatorGreater(std::string s = ">") : OperatorBase("gt", s, GREATER_OPERATOR) {}
    void operate_binary(TokenListBase* tokens) override {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->comparison_greater(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_GREATER_H
