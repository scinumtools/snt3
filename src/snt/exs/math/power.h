#ifndef EXS_OPERATOR_POWER_H
#define EXS_OPERATOR_POWER_H

#include <math.h>

namespace snt::exs {

  template <typename S = EmptySettings>
  class OperatorPower : public OperatorBase<S> {
  public:
    OperatorPower(std::string s = "**") : OperatorBase<S>("pow", s, POWER_OPERATOR) {}
    void operate_binary(TokenListBase* tokens) override {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->math_power(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_POWER_H
