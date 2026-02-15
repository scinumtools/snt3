#ifndef EXS_OPERATOR_CONDITION_H
#define EXS_OPERATOR_CONDITION_H

#include <snt/exs/operator_ternary.h>

namespace snt::exs {

  class OperatorCondition : public OperatorTernary {
  public:
    OperatorCondition();
    void operate_ternary(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_CONDITION_H
