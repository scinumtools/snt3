#include "calculator_atom.h"

#include "../exceptions.h"

#include <algorithm>
#include <regex>
#include <sstream>

namespace snt::puq {

  Quantity CalculatorAtom::from_string(std::string& expr_orig, exs::BaseSettings* set) {
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
    std::stringstream ss;
    if constexpr (Config::debug_calculator) {
      ss << "CALC:    " << value.to_string() << " + " << other->value.to_string() << " = ";
    }
    value += other->value;
    if constexpr (Config::debug_calculator) {
      std::clog << ss.str() << value.to_string() << std::endl;
    }
  }

  void CalculatorAtom::math_subtract(CalculatorAtom* other) {
    std::stringstream ss;
    if constexpr (Config::debug_calculator) {
      ss << "CALC:    " << value.to_string() << " - " << other->value.to_string() << " = ";
    }
    value -= other->value;
    if constexpr (Config::debug_calculator) {
      std::clog << ss.str() << value.to_string() << std::endl;
    }
  }

  void CalculatorAtom::math_negate() {
    if constexpr (Config::debug_calculator) {
      std::clog << "CALC:    - " << value.to_string() << " = ";
    }
    value = -value;
    if constexpr (Config::debug_calculator) {
      std::clog << value.to_string() << std::endl;
    }
  }

  void CalculatorAtom::math_multiply(CalculatorAtom* other) {
    if constexpr (Config::debug_calculator) {
      std::clog << "CALC:    " << value.to_string() << " * " << other->value.to_string() << " = ";
    }
    value *= other->value;
    if constexpr (Config::debug_calculator) {
      std::clog << value.to_string() << std::endl;
    }
  }

  void CalculatorAtom::math_divide(CalculatorAtom* other) {
    if constexpr (Config::debug_calculator) {
      std::clog << "CALC:    " << value.to_string() << " / " << other->value.to_string() << " = ";
    }
    value /= other->value;
    if constexpr (Config::debug_calculator) {
      std::clog << value.to_string() << std::endl;
    }
  }

} // namespace snt::puq
