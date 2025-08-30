#ifndef EXS_SETTINGS_H
#define EXS_SETTINGS_H

#include <string>

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
    CUBIC_ROOT_OPERATOR,
    SINUS_OPERATOR,
    COSINUS_OPERATOR,
    TANGENS_OPERATOR,

    NUM_OPERATOR_TYPES
  };

  static std::string trim(const std::string& str, const std::string& whitespace = " \t") {
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
      return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
  }
  
  struct EmptySettings {};

} // namespace exs

#endif // EXS_SETTINGS_H
