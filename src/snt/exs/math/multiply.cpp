#include "multiply.h"

namespace snt::exs {

  OperatorMultiply::OperatorMultiply(std::string s) :
    OperatorBase("mul", s, MULTIPLY_OPERATOR) {
  }
  
  void OperatorMultiply::operate_binary(TokenListBase* tokens) {
      Token left = tokens->get_left();
      Token right = tokens->get_right();
      left.atom->math_multiply(right.atom);
      tokens->put_left(left);
  };

} // namespace snt::exs
