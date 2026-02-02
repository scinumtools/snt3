#include "less.h"

namespace snt::exs {

  OperatorLess::OperatorLess(std::string s) :
    OperatorBase("lt", s, LESS_OPERATOR) {
  }
  
  void OperatorLess::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->comparison_less(right.atom);
    tokens->put_left(left);
  };

} // namespace snt::exs
