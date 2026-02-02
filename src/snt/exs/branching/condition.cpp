#include "condition.h"

namespace snt::exs {

  OperatorCondition::OperatorCondition() :
    OperatorTernary("cond", "?", ":", CONDITION_OPERATOR) {
  }
  
  void OperatorCondition::operate_ternary(TokenListBase* tokens) {
    Token group2 = tokens->get_left();
    Token group1 = tokens->get_left();
    Token group3 = tokens->get_right();
    group1.atom->condition(group2.atom, group3.atom);
    tokens->put_left(group1);
  };

}

