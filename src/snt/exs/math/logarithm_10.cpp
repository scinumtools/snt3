#include <snt/exs/math/logarithm_10.h>

namespace snt::exs {

  OperatorLogarithm10::OperatorLogarithm10(const OperatorGroupSybols& s) :
    OperatorGroup<1>("log10", s, LOGARITHM_10_OPERATOR) {
  }

  void OperatorLogarithm10::operate_group(TokenListBase* tokens) {
    Token group1 = tokens->get_left();
    group1.atom->math_logarithm_10();
    tokens->put_left(group1);
  };

} // namespace snt::exs
