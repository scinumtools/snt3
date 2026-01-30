#ifndef EXS_OPERATOR_LESS_EQUAL_H
#define EXS_OPERATOR_LESS_EQUAL_H

namespace snt::exs {

  class OperatorLessEqual : public OperatorBase {
  public:
    OperatorLessEqual(std::string s = "<=") : OperatorBase("le", s, LESS_EQUAL_OPERATOR) {}
    void operate_binary(TokenListBase* tokens) override {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->comparison_less_equal(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_LESS_EQUAL_H
