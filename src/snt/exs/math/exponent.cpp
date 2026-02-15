#include <snt/exs/math/exponent.h>

namespace snt::exs {

  OperatorExponent::OperatorExponent(const OperatorGroupSybols& s) :
    OperatorGroup<1>("exp", s, EXPONENT_OPERATOR) {
  }
  
  void OperatorExponent::operate_group(TokenListBase* tokens) {
    Token group1 = tokens->get_left();
    group1.atom->math_exponent();
    tokens->put_left(group1);
  };

}
