#include <snt/exs/math/tangens.h>

namespace snt::exs {

  OperatorTangens::OperatorTangens(const OperatorGroupSybols& s) :
    OperatorGroup<1>("tan", s, TANGENS_OPERATOR) {
  }
  
  void OperatorTangens::operate_group(TokenListBase* tokens) {
    Token group1 = tokens->get_left();
    group1.atom->math_tangens();
    tokens->put_left(group1);
  };

} // namespace snt::exs
