#include <snt/exs/math/sinus.h>

namespace snt::exs {

  OperatorSinus::OperatorSinus(const OperatorGroupSybols& s) :
    OperatorGroup<1>("sin", s, SINUS_OPERATOR) {
  }
  
  void OperatorSinus::operate_group(TokenListBase* tokens) {
    Token group1 = tokens->get_left();
    group1.atom->math_sinus();
    tokens->put_left(group1);
  };

} // namespace snt::exs
