#include <snt/exs/operators/arithmetic.h>

namespace snt::exs {

  // add

  OperatorAdd::OperatorAdd(std::string s) : OperatorBase("add", s, ADD_OPERATOR) {
  }

  void OperatorAdd::operate_unary(TokenListBase* tokens) {
    // std::cout << "operate_unary" << std::endl;
    // tokens->print(true);
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    if (left.type == EMPTY_TOKEN && right.type == ATOM_TOKEN) {
      // std::cout << "a " << left.to_string() << " " << right.to_string() << std::endl;
      tokens->put_left(right);
    } else if (right.optype == ADD_OPERATOR) {
      // std::cout << "b " << left.to_string() << " " << right.to_string() << std::endl;
      tokens->put_left(left);
      tokens->put_right(right);
    } else if (right.optype == SUBTRACT_OPERATOR) {
      // std::cout << "c" << std::endl;
      tokens->put_left(left);
      tokens->put_right(right);
    } else if (left.type != ATOM_TOKEN && right.type == ATOM_TOKEN) {
      // std::cout << "d" << std::endl;
      tokens->put_left(left);
      tokens->put_right(right);
    } else {
      // std::cout << "e" << std::endl;
      tokens->put_left(left);
      tokens->put_left(Token(OPERATOR_TOKEN, ADD_OPERATOR));
      tokens->put_right(right);
    }
  };

  void OperatorAdd::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->math_add(right.atom);
    tokens->put_left(left);
  };

  // subtract

  OperatorSubtract::OperatorSubtract(std::string s) : OperatorBase("sub", s, SUBTRACT_OPERATOR) {
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

  // multiply

  OperatorMultiply::OperatorMultiply(std::string s) : OperatorBase("mul", s, MULTIPLY_OPERATOR) {
  }

  void OperatorMultiply::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->math_multiply(right.atom);
    tokens->put_left(left);
  };

  // divide

  OperatorDivide::OperatorDivide(std::string s) : OperatorBase("div", s, DIVIDE_OPERATOR) {
  }

  void OperatorDivide::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->math_divide(right.atom);
    tokens->put_left(left);
  };

  // modulo

  OperatorModulo::OperatorModulo(std::string s) : OperatorBase("mod", s, MODULO_OPERATOR) {
  }

  void OperatorModulo::operate_binary(TokenListBase* tokens) {
    Token left = tokens->get_left();
    Token right = tokens->get_right();
    left.atom->math_modulo(right.atom);
    tokens->put_left(left);
  };

} // namespace snt::exs
