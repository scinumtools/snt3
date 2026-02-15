#include <snt/exs/math/power_base.h>

namespace snt::exs {

  OperatorPowerBase::OperatorPowerBase(const OperatorGroupSybols& s) :
    OperatorGroup<2>("powb", s, POWER_BASE_OPERATOR) {
  }
  
  void OperatorPowerBase::operate_group(TokenListBase* tokens) {
    Token group2 = tokens->get_left();
    Token group1 = tokens->get_left();
    group1.atom->math_power_base(group2.atom);
    tokens->put_left(group1);
    };

} // namespace snt::exs
