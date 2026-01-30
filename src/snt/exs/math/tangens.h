#ifndef EXS_OPERATOR_TANGENS_H
#define EXS_OPERATOR_TANGENS_H

#include "../operator_group.h"

namespace snt::exs {

  class OperatorTangens : public OperatorGroup<1> {
  public:
    OperatorTangens(const OperatorGroupSybols& s = {"tan", "(", ")", ","}) : OperatorGroup<1>("tan", s, TANGENS_OPERATOR) {}
    void operate_group(TokenListBase* tokens) override {
      Token group1 = tokens->get_left();
      group1.atom->math_tangens();
      tokens->put_left(group1);
    };
  };

} // namespace snt::exs

#endif // EXS_OPERATOR_TANGENS_H
