#include <snt/exs/operators/trigonometry.h>

namespace snt::exs {

  // sinus

  OperatorSinus::OperatorSinus(const OperatorGroupSybols& s) : OperatorGroup<1>("sin", s, SINUS_OPERATOR) {
  }

  void OperatorSinus::operate_group(TokenListBase* tokens) {
    Token group1 = tokens->get_left();
    group1.atom->math_sinus();
    tokens->put_left(group1);
  };

  // cosinus

  OperatorCosinus::OperatorCosinus(const OperatorGroupSybols& s) : OperatorGroup<1>("cos", s, COSINUS_OPERATOR) {
  }

  void OperatorCosinus::operate_group(TokenListBase* tokens) {
    Token group1 = tokens->get_left();
    group1.atom->math_cosinus();
    tokens->put_left(group1);
  };

  // tangens

  OperatorTangens::OperatorTangens(const OperatorGroupSybols& s) : OperatorGroup<1>("tan", s, TANGENS_OPERATOR) {
  }

  void OperatorTangens::operate_group(TokenListBase* tokens) {
    Token group1 = tokens->get_left();
    group1.atom->math_tangens();
    tokens->put_left(group1);
  };

} // namespace snt::exs
