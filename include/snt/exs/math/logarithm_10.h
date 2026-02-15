#ifndef EXS_OPERATOR_LOGARITHM_10_H
#define EXS_OPERATOR_LOGARITHM_10_H

#include <snt/exs/operator_group.h>

namespace snt::exs {

  class OperatorLogarithm10 : public OperatorGroup<1> {
  public:
    
    OperatorLogarithm10(const OperatorGroupSybols& s = {"log10", "(", ")", ","});
    
    void operate_group(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_LOGARITHM_10_H
