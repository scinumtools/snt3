#include <snt/exs/operators/comparison.h>

namespace snt::exs {

  // equal

  OperatorEqual::OperatorEqual(std::string s) : OperatorBase("eq", s, EQUAL_OPERATOR) {
  }

  void OperatorEqual::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->comparison_equal(right.atom);
    tokens->put_left(left);
  };

  // not equal

  OperatorNotEqual::OperatorNotEqual(std::string s) : OperatorBase("ne", s, NOT_EQUAL_OPERATOR) {
  }

  void OperatorNotEqual::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->comparison_not_equal(right.atom);
    tokens->put_left(left);
  };

  // greater

  OperatorGreater::OperatorGreater(std::string s) : OperatorBase("gt", s, GREATER_OPERATOR) {
  }

  void OperatorGreater::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->comparison_greater(right.atom);
    tokens->put_left(left);
  };

  // less

  OperatorLess::OperatorLess(std::string s) : OperatorBase("lt", s, LESS_OPERATOR) {
  }

  void OperatorLess::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->comparison_less(right.atom);
    tokens->put_left(left);
  };

  // greater equal

  OperatorGreaterEqual::OperatorGreaterEqual(std::string s) : OperatorBase("ge", s, GREATER_EQUAL_OPERATOR) {
  }

  void OperatorGreaterEqual::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->comparison_greater_equal(right.atom);
    tokens->put_left(left);
  };

  // less equal

  OperatorLessEqual::OperatorLessEqual(std::string s) : OperatorBase("le", s, LESS_EQUAL_OPERATOR) {
  }

  void OperatorLessEqual::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->comparison_less_equal(right.atom);
    tokens->put_left(left);
  };

} // namespace snt::exs
