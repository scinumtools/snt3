#include "power.h"

namespace snt::exs {

  OperatorPower::OperatorPower(std::string s) :
    OperatorBase("pow", s, POWER_OPERATOR) {
  }
  
  void OperatorPower::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->math_power(right.atom);
    tokens->put_left(left);
  };

} // namespace snt::exs
