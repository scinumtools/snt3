#ifndef EXS_OPERATOR_LESS_EQUAL_H
#define EXS_OPERATOR_LESS_EQUAL_H

namespace snt::exs {

  template <typename S = EmptySettings>
  class OperatorLessEqual : public OperatorBase<S> {
  public:
    OperatorLessEqual(std::string s = "<=") : OperatorBase<S>("le", s, LESS_EQUAL_OPERATOR) {}
    void operate_binary(TokenListBase* tokens) override {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->comparison_less_equal(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_LESS_EQUAL_H
