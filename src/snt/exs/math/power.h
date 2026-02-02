#ifndef EXS_OPERATOR_POWER_H
#define EXS_OPERATOR_POWER_H

#include "../operator_base.h"

#include <math.h>

namespace snt::exs {

  class OperatorPower : public OperatorBase {
  public:
    
    OperatorPower(std::string s = "**");
    
    void operate_binary(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_POWER_H
