#ifndef EXS_OPERATOR_TANGENS_H
#define EXS_OPERATOR_TANGENS_H

#include "../operator_group.h"

namespace snt::exs {

  template <typename S = EmptySettings>
  class OperatorTangens : public OperatorGroup<1, S> {
  public:
    OperatorTangens(const OperatorGroupSybols& s = {"tan", "(", ")", ","}) : OperatorGroup<1, S>("tan", s, TANGENS_OPERATOR) {}
    void operate_group(TokenListBase* tokens) override {
      Token group1 = tokens->get_left();
      group1.atom->math_tangens();
      tokens->put_left(group1);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_TANGENS_H
