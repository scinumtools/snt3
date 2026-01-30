#ifndef EXS_OPERATOR_PARENTHESES_H
#define EXS_OPERATOR_PARENTHESES_H

#include "../operator_group.h"

namespace snt::exs {

  class OperatorParentheses : public OperatorGroup<1> {
  public:
    OperatorParentheses(const OperatorGroupSybols& s = {"", "(", ")", ","}) : OperatorGroup<1>("par", s, PARENTHESES_OPERATOR) {}
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_PARENTHESES_H
