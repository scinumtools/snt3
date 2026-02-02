#ifndef EXS_OPERATOR_TANGENS_H
#define EXS_OPERATOR_TANGENS_H

#include "../operator_group.h"

namespace snt::exs {

  class OperatorTangens : public OperatorGroup<1> {
  public:
    
    OperatorTangens(const OperatorGroupSybols& s = {"tan", "(", ")", ","});
    
    void operate_group(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_TANGENS_H
