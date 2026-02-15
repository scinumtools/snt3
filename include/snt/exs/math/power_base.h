#ifndef EXS_OPERATOR_POWER_BASE_H
#define EXS_OPERATOR_POWER_BASE_H

#include <snt/exs/operator_group.h>

namespace snt::exs {

  class OperatorPowerBase : public OperatorGroup<2> {
  public:
    
    OperatorPowerBase(const OperatorGroupSybols& s = {"powb", "(", ")", ","});
    
    void operate_group(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_POWER_BASE_H
