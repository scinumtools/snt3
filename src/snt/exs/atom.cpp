#include <snt/exs/atom.h>

namespace snt::exs {

  AtomValueType Atom::from_string(std::string s, BaseSettings* set) {
    AtomValueType v;
    if (s == "true") {
      v = true;
    } else if (s == "false") {
      v = false;
    } else {
      std::regex rx("((\\+|-)?[[:digit:]]+)(\\.(([[:digit:]]+)?))?((e|E)((\\+|-)?)[[:digit:]]+)?");
      if (std::regex_match(s, rx)) {
	v = std::stof(s);
      } else {
	throw std::logic_error("Atom string could not be parsed, probably due to unknown symbol or operator: " + s);
      }
    }
    return v;
  }
  
  std::string Atom::to_string() {
    if (std::holds_alternative<double>(value)) {
      std::stringstream str;
      str << std::get<double>(value) << std::scientific;
      return str.str();
    } else {
      if (std::get<bool>(value) == 0)
	return "false";
      else
	return "true";
    }
  }

  // Math operations
  void Atom::math_add(Atom* other) {
    value = std::get<double>(value) + std::get<double>(other->value);
  }
  void Atom::math_subtract(Atom* other) {
    value = std::get<double>(value) - std::get<double>(other->value);
  }
  void Atom::math_negate() {
    value = -std::get<double>(value);
  }
  void Atom::math_multiply(Atom* other) {
    value = std::get<double>(value) * std::get<double>(other->value);
  }
  void Atom::math_divide(Atom* other) {
    value = std::get<double>(value) / std::get<double>(other->value);
  }
  void Atom::math_power(Atom* other) {
    value = pow(std::get<double>(value), std::get<double>(other->value));
  }
  void Atom::math_modulo(Atom* other) {
    value = (double)((int)std::get<double>(value) % (int)std::get<double>(other->value));
  }

  // Argument math operators
  void Atom::math_exponent() {
    value = std::exp(std::get<double>(value));
  }
  void Atom::math_logarithm() {
    value = std::log(std::get<double>(value));
  }
  void Atom::math_logarithm_10() {
    value = std::log10(std::get<double>(value));
  }
  void Atom::math_logarithm_base(Atom* other) {
    value = std::log(std::get<double>(value)) / std::log(std::get<double>(other->value));
  }
  void Atom::math_power_base(Atom* other) {
    value = pow(std::get<double>(value), std::get<double>(other->value));
  }
  void Atom::math_square_root() {
    value = std::sqrt(std::get<double>(value));
  }
  void Atom::math_cubic_root() {
    value = std::cbrt(std::get<double>(value));
  }
  void Atom::math_sinus() {
    value = std::sin(std::get<double>(value));
  }
  void Atom::math_cosinus() {
    value = std::cos(std::get<double>(value));
  }
  void Atom::math_tangens() {
    value = std::tan(std::get<double>(value));
  }

  // Comparison operations
  void Atom::comparison_equal(Atom* other) {
    value = std::get<double>(value) == std::get<double>(other->value);
  }
  void Atom::comparison_not_equal(Atom* other) {
    value = std::get<double>(value) != std::get<double>(other->value);
  }
  void Atom::comparison_less_equal(Atom* other) {
    value = std::get<double>(value) <= std::get<double>(other->value);
  }
  void Atom::comparison_greater_equal(Atom* other) {
    value = std::get<double>(value) >= std::get<double>(other->value);
  }
  void Atom::comparison_less(Atom* other) {
    value = std::get<double>(value) < std::get<double>(other->value);
  }
  void Atom::comparison_greater(Atom* other) {
    value = std::get<double>(value) > std::get<double>(other->value);
  }

  // Logical operations
  void Atom::logical_not() {
    value = !std::get<bool>(value);
  }
  void Atom::logical_and(Atom* other) {
    value = std::get<bool>(value) && std::get<bool>(other->value);
  }
  void Atom::logical_or(Atom* other) {
    value = std::get<bool>(value) || std::get<bool>(other->value);
  }

  // Conditional operator
  void Atom::condition(Atom* option1, Atom* option2) {
    value = std::get<bool>(value) ? std::get<double>(option1->value) : std::get<double>(option2->value);
  }
  
}
