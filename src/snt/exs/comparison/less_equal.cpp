#include <snt/exs/comparison/less_equal.h>

namespace snt::exs {

  OperatorLessEqual::OperatorLessEqual(std::string s) :
    OperatorBase("le", s, LESS_EQUAL_OPERATOR) {
  }
  
  void OperatorLessEqual::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->comparison_less_equal(right.atom);
    tokens->put_left(left);
  };

} // namespace snt::exs
