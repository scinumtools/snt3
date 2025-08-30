#include "operator_parentheses.h"

#include "unit_atom.h"

namespace puq {

  bool OperatorParentheses::check(exs::Expression& expr) {
    if (expr.left.size() > 0) {
      char c = expr.left.back();
      if (std::isdigit(c) || c == '.') {
        return false; // ignore parentheses if in the numerical value
      }
    }
    return expr.right.rfind(symbol, 0) == 0;
  }

  void OperatorParentheses::parse(exs::Expression& expr) {
    OperatorGroup<UnitAtom, 1>::parse(expr); // perform ordinary parsing
    if (expr.right.length() > 0) {           // check if there is an exponent after closing parentheses
      std::smatch m;
#ifdef EXPONENT_FRACTIONS
      std::regex rx_exponent("^([+-]?[0-9]*)(" + std::string(SYMBOL_FRACTION) + "([0-9]+)|)");
      if (std::regex_search(expr.right, m, rx_exponent)) { // store exponent
        exponent.push_back(Exponent((m[1] == "" ? 1 : std::stoi(m[1])), (m[3] == "" ? 1 : std::stoi(m[3]))));
        expr.remove(m[0]);
      } else {
        exponent.push_back(1);
      }
#else
      std::regex rx_exponent("^([+-]?[0-9]*)");
      if (std::regex_search(expr.right, m, rx_exponent)) { // store exponent
        if (m[1] != "") {
          exponent.push_back(std::stoi(m[1]));
        } else {
          exponent.push_back(1);
        }
        expr.remove(m[0]);
      }
#endif
    } else {
      exponent.push_back(1);
    }
  };

  void OperatorParentheses::operate_group(exs::TokenListBase<UnitAtom>* tokens) {
    if (exponent.size() == 0)
      throw AtomParsingExcept("Number of exponents does not match number of opened parentheses!");
    EXPONENT_TYPE exp = exponent.back();
    exponent.pop_back();
    if (exp != 1) {
      exs::Token<UnitAtom> group1 = tokens->get_left();
      group1.atom->math_power(exp);
      tokens->put_left(group1);
    }
  };

} // namespace puq
