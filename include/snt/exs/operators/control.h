#ifndef EXS_OPERATOR_CONTROL_H
#define EXS_OPERATOR_CONTROL_H

#include <snt/exs/operator_group.h>
#include <snt/exs/operator_ternary.h>

namespace snt::exs {

  // parentheses

  class OperatorParentheses : public OperatorGroup<1> {
  public:
    OperatorParentheses(const OperatorGroupSybols& s = {"", "(", ")", ","});
  };

  // conditional operator

  class OperatorCondition : public OperatorTernary {
  public:
    OperatorCondition();
    void operate_ternary(TokenListBase* tokens) override;
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_CONTROL_H
