#ifndef EXS_OPERATOR_TANGENS_H
#define EXS_OPERATOR_TANGENS_H

#include "../operator_group.h"

namespace exs {

  template <class A, typename S = EmptySettings>
  class OperatorTangens : public OperatorGroup<A, 1, S> {
  public:
    OperatorTangens(const OperatorGroupSybols& s = {"tan", "(", ")", ","}) : OperatorGroup<A, 1, S>("tan", s, TANGENS_OPERATOR) {}
    void operate_group(TokenListBase<A>* tokens) override {
      Token<A> group1 = tokens->get_left();
      group1.atom->math_tangens();
      tokens->put_left(group1);
    };
  };

} // namespace exs

#endif // EXS_OPERATOR_TANGENS_H
