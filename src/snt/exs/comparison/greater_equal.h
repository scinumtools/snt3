#ifndef EXS_OPERATOR_GREATER_EQUAL_H
#define EXS_OPERATOR_GREATER_EQUAL_H

namespace snt::exs {

  template <typename S = EmptySettings>
  class OperatorGreaterEqual : public OperatorBase<S> {
  public:
    OperatorGreaterEqual(std::string s = ">=") : OperatorBase<S>("ge", s, GREATER_EQUAL_OPERATOR) {}
    void operate_binary(TokenListBase* tokens) override {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->comparison_greater_equal(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_GREATER_EQUAL_H
