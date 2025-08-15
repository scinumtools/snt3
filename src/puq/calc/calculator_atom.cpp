#include <algorithm>
#include <regex>
#include <sstream>

#include "../nostd/nostd.h"
#include "../value/value.h"
#include "calc.h"

namespace puq {

  Quantity CalculatorAtom::from_string(std::string& expr_orig) {
    std::string& expr = expr_orig;
    std::smatch m;
    std::regex rx_number("^([^\\s]+)$");
    if (std::regex_match(expr, m, rx_number)) {
      return Quantity(expr);
    } else {
      throw CalculatorExcept("Invalid unit expression: " + expr_orig);
    }
  }

  std::string CalculatorAtom::to_string() {
    return value.to_string();
  }

  void CalculatorAtom::math_add(CalculatorAtom* other) {
#ifdef DEBUG_CALCULATOR
    std::stringstream ss;
    ss << "CALC:    " << value.to_string() << " + " << other->value.to_string() << " = ";
#endif
    value += other->value;
#ifdef DEBUG_CALCULATOR
    std::clog << ss.str() << value.to_string() << std::endl;
    ;
#endif
  }

  void CalculatorAtom::math_subtract(CalculatorAtom* other) {
#ifdef DEBUG_CALCULATOR
    std::stringstream ss;
    ss << "CALC:    " << value.to_string() << " - " << other->value.to_string() << " = ";
#endif
    value -= other->value;
#ifdef DEBUG_CALCULATOR
    std::clog << ss.str() << value.to_string() << std::endl;
    ;
#endif
  }

  void CalculatorAtom::math_negate() {
#ifdef DEBUG_CALCULATOR
    std::clog << "CALC:    - " << value.to_string() << " = ";
#endif
    value = -value;
#ifdef DEBUG_CALCULATOR
    std::clog << value.to_string() << std::endl;
#endif
  }

  void CalculatorAtom::math_multiply(CalculatorAtom* other) {
#ifdef DEBUG_CALCULATOR
    std::clog << "CALC:    " << value.to_string() << " * " << other->value.to_string() << " = ";
#endif
    value *= other->value;
#ifdef DEBUG_CALCULATOR
    std::clog << value.to_string() << std::endl;
#endif
  }

  void CalculatorAtom::math_divide(CalculatorAtom* other) {
#ifdef DEBUG_CALCULATOR
    std::clog << "CALC:    " << value.to_string() << " / " << other->value.to_string() << " = ";
#endif
    value /= other->value;
#ifdef DEBUG_CALCULATOR
    std::clog << value.to_string() << std::endl;
#endif
  }

} // namespace puq
