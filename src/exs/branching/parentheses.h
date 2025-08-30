#ifndef EXS_OPERATOR_PARENTHESES_H
#define EXS_OPERATOR_PARENTHESES_H

#include "../operator_group.h"

namespace exs {

  template <class A, typename S = EmptySettings>
  class OperatorParentheses : public OperatorGroup<A, 1, S> {
  public:
    OperatorParentheses(const OperatorGroupSybols& s = {"", "(", ")", ","}) : OperatorGroup<A, 1, S>("par", s, PARENTHESES_OPERATOR) {}
  };

} // namespace exs

#endif // EXS_OPERATOR_PARENTHESES_H
