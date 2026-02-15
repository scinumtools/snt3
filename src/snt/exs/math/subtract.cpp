#include <snt/exs/math/subtract.h>

namespace snt::exs {
  
  OperatorSubtract::OperatorSubtract(std::string s) :
    OperatorBase("sub", s, SUBTRACT_OPERATOR) {
  }
  
  void OperatorSubtract::operate_unary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    if (left.type == EMPTY_TOKEN && right.type == ATOM_TOKEN) {
      right.atom->math_negate();
      tokens->put_right(right);
    } else if (right.optype == ADD_OPERATOR) {
      tokens->put_left(left);
      tokens->put_right(Token(OPERATOR_TOKEN, SUBTRACT_OPERATOR));
    } else if (right.optype == SUBTRACT_OPERATOR) {
      tokens->put_left(left);
      tokens->put_right(Token(OPERATOR_TOKEN, ADD_OPERATOR));
    } else if (left.type != ATOM_TOKEN && right.type == ATOM_TOKEN) {
      tokens->put_left(left);
      right.atom->math_negate();
      tokens->put_right(right);
    } else {
      tokens->put_left(left);
      tokens->put_left(Token(OPERATOR_TOKEN, SUBTRACT_OPERATOR));
      tokens->put_right(right);
    }
  };
  
  void OperatorSubtract::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->math_subtract(right.atom);
    tokens->put_left(left);
  };

} // namespace snt::exs
