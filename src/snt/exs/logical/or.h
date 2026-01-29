#ifndef EXS_OPERATOR_OR_H
#define EXS_OPERATOR_OR_H

namespace snt::exs {

  template <typename S = EmptySettings>
  class OperatorOr : public OperatorBase<S> {
  public:
    OperatorOr(std::string s = "||") : OperatorBase<S>("or", s, OR_OPERATOR) {}
    void operate_binary(TokenListBase* tokens) override {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->logical_or(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_OR_H
