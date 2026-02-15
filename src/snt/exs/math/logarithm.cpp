#include <snt/exs/math/logarithm.h>

namespace snt::exs {

  OperatorLogarithm::OperatorLogarithm(const OperatorGroupSybols& s) :
    OperatorGroup<1>("log", s, LOGARITHM_OPERATOR) {
  }
  
  void OperatorLogarithm::operate_group(TokenListBase* tokens) {
    Token group1 = tokens->get_left();
    group1.atom->math_logarithm();
    tokens->put_left(group1);
  };
  
}
