#include <snt/exs/operators/logical.h>

namespace snt::exs {

  // and
  
  OperatorAnd::OperatorAnd(std::string s) :
    OperatorBase("and", s, AND_OPERATOR) {
  }
  
  void OperatorAnd::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->logical_and(right.atom);
    tokens->put_left(left);
  };

  // or
  
  OperatorOr::OperatorOr(std::string s) :
    OperatorBase("or", s, OR_OPERATOR) {
  }
  
  void OperatorOr::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->logical_or(right.atom);
    tokens->put_left(left);
  };

  // not
  
  OperatorNot::OperatorNot(std::string s) :
    OperatorBase("not", s, NOT_OPERATOR) {
  }
  
  void OperatorNot::operate_unary(TokenListBase* tokens) {
    Token right = tokens->get_right();
    right.atom->logical_not();
    tokens->put_right(right);
  };
  
}

