#ifndef EXS_MAIN_H
#define EXS_MAIN_H

#include <any>
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace exs {

  enum TokenType {
    EMPTY_TOKEN,
    OPERATOR_TOKEN,
    ATOM_TOKEN,
  };

  enum OperationType {
    UNARY_OPERATION,
    BINARY_OPERATION,
    TERNARY_OPERATION,
    GROUP_OPERATION,
  };

  enum OperatorType {
    NONE_OPERATOR,

    ADD_OPERATOR,
    SUBTRACT_OPERATOR,
    MULTIPLY_OPERATOR,
    DIVIDE_OPERATOR,
    POWER_OPERATOR,
    MODULO_OPERATOR,

    NOT_OPERATOR,
    AND_OPERATOR,
    OR_OPERATOR,

    EQUAL_OPERATOR,
    NOT_EQUAL_OPERATOR,
    LESS_EQUAL_OPERATOR,
    GREATER_EQUAL_OPERATOR,
    LESS_OPERATOR,
    GREATER_OPERATOR,

    CONDITION_OPERATOR,

    PARENTHESES_OPERATOR,
    EXPONENT_OPERATOR,
    LOGARITHM_OPERATOR,
    LOGARITHM_10_OPERATOR,
    LOGARITHM_BASE_OPERATOR,
    POWER_BASE_OPERATOR,
    SQUARE_ROOT_OPERATOR,
    SINUS_OPERATOR,
    COSINUS_OPERATOR,
    TANGENS_OPERATOR,

    NUM_OPERATOR_TYPES
  };

} // namespace exs

#include "atom.h"
#include "atom_list.h"
#include "token.h"

namespace exs {

  template <class A>
  class TokenListBase {
  public:
    virtual Token<A> get_left() = 0;
    virtual Token<A> get_right() = 0;
    virtual void put_left(Token<A> t) = 0;
    virtual void put_right(Token<A> t) = 0;
    virtual void print(bool details = false) = 0;
  };

  static std::string trim(const std::string& str, const std::string& whitespace = " \t") {
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
      return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
  }

} // namespace exs

#include "expression.h"
#include "operator_list.h"
#include "operators.h"
#include "solver.h"
#include "step_list.h"
#include "token_list.h"

#endif // EXS_MAIN_H
