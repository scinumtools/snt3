#ifndef EXS_OPERATOR_EXPONENT_H
#define EXS_OPERATOR_EXPONENT_H

#include <snt/exs/operator_group.h>

namespace snt::exs {

  class OperatorExponent : public OperatorGroup<1> {
  public:
    
    OperatorExponent(const OperatorGroupSybols& s = {"exp", "(", ")", ","});
    
    void operate_group(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_EXPONENT_H
