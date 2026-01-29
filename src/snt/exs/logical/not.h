#ifndef EXS_OPERATOR_NOT_H
#define EXS_OPERATOR_NOT_H

namespace snt::exs {

  template <typename S = EmptySettings>
  class OperatorNot : public OperatorBase<S> {
  public:
    OperatorNot(std::string s = "!") : OperatorBase<S>("not", s, NOT_OPERATOR) {}
    void operate_unary(TokenListBase* tokens) override {
      Token right = tokens->get_right();
      right.atom->logical_not();
      tokens->put_right(right);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_NOT_H
