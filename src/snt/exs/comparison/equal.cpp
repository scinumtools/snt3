#include <snt/exs/comparison/equal.h>

namespace snt::exs {

  OperatorEqual::OperatorEqual(std::string s) :
    OperatorBase("eq", s, EQUAL_OPERATOR) {
  }

  void OperatorEqual::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->comparison_equal(right.atom);
    tokens->put_left(left);
  };

} // namespace snt::exs
