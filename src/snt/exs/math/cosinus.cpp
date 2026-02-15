#include <snt/exs/math/cosinus.h>

namespace snt::exs {

  OperatorCosinus::OperatorCosinus(const OperatorGroupSybols& s) :
    OperatorGroup<1>("cos", s, COSINUS_OPERATOR) {
  }
  
  void OperatorCosinus::operate_group(TokenListBase* tokens) {
    Token group1 = tokens->get_left();
    group1.atom->math_cosinus();
    tokens->put_left(group1);
  };
  
}
