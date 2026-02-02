#ifndef EXS_OPERATOR_SINUS_H
#define EXS_OPERATOR_SINUS_H

#include "../operator_group.h"

namespace snt::exs {

  class OperatorSinus : public OperatorGroup<1> {
  public:
    
    OperatorSinus(const OperatorGroupSybols& s = {"sin", "(", ")", ","});
    
    void operate_group(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_SINUS_H
