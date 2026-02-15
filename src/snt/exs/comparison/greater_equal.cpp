#include <snt/exs/comparison/greater_equal.h>

namespace snt::exs {

  OperatorGreaterEqual::OperatorGreaterEqual(std::string s) :
    OperatorBase("ge", s, GREATER_EQUAL_OPERATOR) {
  }
  
  void OperatorGreaterEqual::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->comparison_greater_equal(right.atom);
    tokens->put_left(left);
  };

} // namespace snt::exs
