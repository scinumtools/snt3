#ifndef EXS_OPERATOR_AND_H
#define EXS_OPERATOR_AND_H

#include "../operator_base.h"

namespace snt::exs {

  class OperatorAnd : public OperatorBase {
  public:
    OperatorAnd(std::string s = "&&") : OperatorBase("and", s, AND_OPERATOR) {}
    void operate_binary(TokenListBase* tokens) override {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->logical_and(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_AND_H
