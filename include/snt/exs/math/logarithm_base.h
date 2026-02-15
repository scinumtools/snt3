#ifndef EXS_OPERATOR_LOGARITHM_BASE_H
#define EXS_OPERATOR_LOGARITHM_BASE_H

#include <snt/exs/operator_group.h>

namespace snt::exs {

  class OperatorLogarithmBase : public OperatorGroup<2> {
  public:
    
    OperatorLogarithmBase(const OperatorGroupSybols& s = {"logb", "(", ")", ","});
    
    void operate_group(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_LOGARITHM_BASE_H
