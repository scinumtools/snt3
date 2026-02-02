#include "divide.h"

namespace snt::exs {

  OperatorDivide::OperatorDivide(std::string s):
    OperatorBase("div", s, DIVIDE_OPERATOR) {
  }
  
  void OperatorDivide::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->math_divide(right.atom);
    tokens->put_left(left);
  };
  
}
