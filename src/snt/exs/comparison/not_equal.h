#ifndef EXS_OPERATOR_NOT_EQUAL_H
#define EXS_OPERATOR_NOT_EQUAL_H

namespace snt::exs {

  class OperatorNotEqual : public OperatorBase {
  public:
    OperatorNotEqual(std::string s = "!=") : OperatorBase("ne", s, NOT_EQUAL_OPERATOR) {}
    void operate_binary(TokenListBase* tokens) override {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->comparison_not_equal(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_NOT_EQUAL_H
