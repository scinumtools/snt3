#ifndef EXS_OPERATOR_EQUAL_H
#define EXS_OPERATOR_EQUAL_H

namespace snt::exs {

  class OperatorEqual : public OperatorBase {
  public:
    OperatorEqual(std::string s = "==") : OperatorBase("eq", s, EQUAL_OPERATOR) {}
    void operate_binary(TokenListBase* tokens) override {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->comparison_equal(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_EQUAL_H
