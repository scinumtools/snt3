#ifndef EXS_OPERATOR_POWER_H
#define EXS_OPERATOR_POWER_H

#include <math.h>

namespace snt::exs {

  template <class A, typename S = EmptySettings>
  class OperatorPower : public OperatorBase<A, S> {
  public:
    OperatorPower(std::string s = "**") : OperatorBase<A, S>("pow", s, POWER_OPERATOR) {}
    void operate_binary(TokenListBase<A>* tokens) override {
      Token<A> left = tokens->get_left();
      Token<A> right = tokens->get_right();
      left.atom->math_power(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_POWER_H
