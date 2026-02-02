#include "logarithm_base.h"

namespace snt::exs {

  OperatorLogarithmBase::OperatorLogarithmBase(const OperatorGroupSybols& s) :
    OperatorGroup<2>("logb", s, LOGARITHM_BASE_OPERATOR) {
  }

  void OperatorLogarithmBase::operate_group(TokenListBase* tokens) {
    Token group2 = tokens->get_left();
    Token group1 = tokens->get_left();
    group1.atom->math_logarithm_base(group2.atom);
    tokens->put_left(group1);
  };

} // namespace snt::exs

