#include "not.h"

namespace snt::exs {

  OperatorNot::OperatorNot(std::string s) :
    OperatorBase("not", s, NOT_OPERATOR) {
  }
  
  void OperatorNot::operate_unary(TokenListBase* tokens) {
    Token right = tokens->get_right();
    right.atom->logical_not();
    tokens->put_right(right);
  };
  
}
