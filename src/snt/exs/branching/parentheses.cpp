#include "parentheses.h"

namespace snt::exs {

  OperatorParentheses::OperatorParentheses(const OperatorGroupSybols& s) :
      OperatorGroup<1>("par", s, PARENTHESES_OPERATOR) {
    }
  
}
