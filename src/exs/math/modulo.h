#ifndef EXS_OPERATOR_MODULO_H
#define EXS_OPERATOR_MODULO_H

namespace snt::exs {

  template <class A, typename S = EmptySettings>
  class OperatorModulo : public OperatorBase<A, S> {
  public:
    OperatorModulo(std::string s = "%") : OperatorBase<A, S>("mod", s, MODULO_OPERATOR) {}
    void operate_binary(TokenListBase<A>* tokens) override {
      Token<A> left = tokens->get_left();
      Token<A> right = tokens->get_right();
      left.atom->math_modulo(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_MODULO_H
