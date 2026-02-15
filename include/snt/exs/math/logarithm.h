#ifndef EXS_OPERATOR_LOGARITHM_H
#define EXS_OPERATOR_LOGARITHM_H

#include <snt/exs/operator_group.h>

namespace snt::exs {

  class OperatorLogarithm : public OperatorGroup<1> {
  public:
    
    OperatorLogarithm(const OperatorGroupSybols& s = {"log", "(", ")", ","});
    
    void operate_group(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_LOGARITHM_H
