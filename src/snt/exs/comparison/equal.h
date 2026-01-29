#ifndef EXS_OPERATOR_EQUAL_H
#define EXS_OPERATOR_EQUAL_H

namespace snt::exs {

  template <typename S = EmptySettings>
  class OperatorEqual : public OperatorBase<S> {
  public:
    OperatorEqual(std::string s = "==") : OperatorBase<S>("eq", s, EQUAL_OPERATOR) {}
    void operate_binary(TokenListBase* tokens) override {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->comparison_equal(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_EQUAL_H
