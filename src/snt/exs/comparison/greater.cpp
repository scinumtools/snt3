#include <snt/exs/comparison/greater.h>

namespace snt::exs {

  OperatorGreater::OperatorGreater(std::string s) :
    OperatorBase("gt", s, GREATER_OPERATOR) {
  }
  
  void OperatorGreater::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->comparison_greater(right.atom);
    tokens->put_left(left);
  };

} // namespace snt::exs
