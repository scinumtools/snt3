#ifndef EXS_OPERATOR_CONDITION_H
#define EXS_OPERATOR_CONDITION_H

#include "../operator_ternary.h"

namespace snt::exs {

  class OperatorCondition : public OperatorTernary {
  public:
    OperatorCondition() : OperatorTernary("cond", "?", ":", CONDITION_OPERATOR) {}
    void operate_ternary(TokenListBase* tokens) override {
      Token group2 = tokens->get_left();
      Token group1 = tokens->get_left();
      Token group3 = tokens->get_right();
      group1.atom->condition(group2.atom, group3.atom);
      tokens->put_left(group1);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_CONDITION_H
