#include <snt/exs/math/cubic_root.h>

namespace snt::exs {

  OperatorCubicRoot::OperatorCubicRoot(const OperatorGroupSybols& s) :
    OperatorGroup<1>("cbrt", s, CUBIC_ROOT_OPERATOR) {
  }
  
  void OperatorCubicRoot::operate_group(TokenListBase* tokens) {
    Token group1 = tokens->get_left();
    group1.atom->math_cubic_root();
    tokens->put_left(group1);
  };

}
