#include "and.h"

namespace snt::exs {

  OperatorAnd::OperatorAnd(std::string s) :
    OperatorBase("and", s, AND_OPERATOR) {
  }
  
  void OperatorAnd::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->logical_and(right.atom);
    tokens->put_left(left);
  };

  
}
