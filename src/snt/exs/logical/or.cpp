#include <snt/exs/logical/or.h>

namespace snt::exs {

  OperatorOr::OperatorOr(std::string s) :
    OperatorBase("or", s, OR_OPERATOR) {
  }
  
  void OperatorOr::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->logical_or(right.atom);
    tokens->put_left(left);
  };

}
