#include <snt/exs/comparison/not_equal.h>

namespace snt::exs {

  OperatorNotEqual::OperatorNotEqual(std::string s) :
    OperatorBase("ne", s, NOT_EQUAL_OPERATOR) {
  }
  
  void OperatorNotEqual::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->comparison_not_equal(right.atom);
    tokens->put_left(left);
  };

} // namespace snt::exs
