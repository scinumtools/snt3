#include "square_root.h"

namespace snt::exs {

  OperatorSquareRoot::OperatorSquareRoot(const OperatorGroupSybols& s) :
    OperatorGroup<1>("sqrt", s, SQUARE_ROOT_OPERATOR) {
  }
  
  void OperatorSquareRoot::operate_group(TokenListBase* tokens) {
    Token group1 = tokens->get_left();
    group1.atom->math_square_root();
    tokens->put_left(group1);
  };

} // namespace snt::exs
