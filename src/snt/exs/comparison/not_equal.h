#ifndef EXS_OPERATOR_NOT_EQUAL_H
#define EXS_OPERATOR_NOT_EQUAL_H

namespace snt::exs {

  template <typename S = EmptySettings>
  class OperatorNotEqual : public OperatorBase<S> {
  public:
    OperatorNotEqual(std::string s = "!=") : OperatorBase<S>("ne", s, NOT_EQUAL_OPERATOR) {}
    void operate_binary(TokenListBase* tokens) override {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->comparison_not_equal(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_NOT_EQUAL_H
