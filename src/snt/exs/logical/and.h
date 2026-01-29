#ifndef EXS_OPERATOR_AND_H
#define EXS_OPERATOR_AND_H

#include "../operator_base.h"

namespace snt::exs {

  template <typename S = EmptySettings>
  class OperatorAnd : public OperatorBase<S> {
  public:
    OperatorAnd(std::string s = "&&") : OperatorBase<S>("and", s, AND_OPERATOR) {}
    void operate_binary(TokenListBase* tokens) override {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->logical_and(right.atom);
      tokens->put_left(left);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_AND_H
