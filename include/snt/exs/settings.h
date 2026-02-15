#ifndef EXS_SETTINGS_H
#define EXS_SETTINGS_H

#include <string>

namespace snt::exs {

  /**
   * @enum TokenType
   * @brief List of operator types
   */
  enum TokenType {
    EMPTY_TOKEN,       ///< Token is empty
    OPERATOR_TOKEN,    ///< Token is an operator
    ATOM_TOKEN,        ///< Token is an atom
  };

  /**
   * @enum OperationType
   * @brief Type of an operation
   */
  enum OperationType {
    UNARY_OPERATION,    ///< Unary operation operates on the atom value
    BINARY_OPERATION,   ///< Binary operation operates between two atoms
    TERNARY_OPERATION,  ///< Ternary operation operates with three atoms
    GROUP_OPERATION,    ///< Group operation operates on various number of atoms
  };

  /**
   * @enum OperatorTree
   * @brief List of all operators
   */
  enum OperatorType {
    NONE_OPERATOR,           ///< Item is not an operator
			     
    ADD_OPERATOR,            ///< Addition of values
    SUBTRACT_OPERATOR,       ///< Subtraction of values
    MULTIPLY_OPERATOR,       ///< Multiplication of values
    DIVIDE_OPERATOR,         ///< Division of values
    POWER_OPERATOR,          ///< Power of a value
    MODULO_OPERATOR,         ///< Modulo of values

    NOT_OPERATOR,            ///< Logical negation
    AND_OPERATOR,            ///< Logical and
    OR_OPERATOR,             ///< Logical or

    EQUAL_OPERATOR,          ///< Logical equality
    NOT_EQUAL_OPERATOR,      ///< Logical difference
    LESS_EQUAL_OPERATOR,     ///< Logical less or equal
    GREATER_EQUAL_OPERATOR,  ///< Logical greater or equal
    LESS_OPERATOR,           ///< Logical less
    GREATER_OPERATOR,        ///< Logical greater

    CONDITION_OPERATOR,      ///< Conditional operator

    PARENTHESES_OPERATOR,    ///< Parentheses
    EXPONENT_OPERATOR,       ///< Exponents 
    LOGARITHM_OPERATOR,      ///< Logarithm
    LOGARITHM_10_OPERATOR,   ///< Logarithm with a base 10
    LOGARITHM_BASE_OPERATOR, ///< Logarithm with an arbitrary base
    POWER_BASE_OPERATOR,     ///< Power with an arbitrary base
    SQUARE_ROOT_OPERATOR,    ///< Square root
    CUBIC_ROOT_OPERATOR,     ///< Cubic root
    SINUS_OPERATOR,          ///< Sinus
    COSINUS_OPERATOR,        ///< Cosinus
    TANGENS_OPERATOR,        ///< Tangens

    NUM_OPERATOR_TYPES       ///< Total number of operators
  };

  /**
   * @brief Remove leading and trailing spaces from a string
   *
   * @param str String that should be trimmed
   * @param whitespace String with whitespace characters that will be removed
   */
  static std::string trim(const std::string& str, const std::string& whitespace = " \t") {
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
      return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
  }

  /**
   * @struct BaseSettings
   * @brief Base structure for expression solver settings
   */
  struct BaseSettings {};

} // namespace snt::exs

#endif // EXS_SETTINGS_H
