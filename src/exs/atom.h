#ifndef EXS_ATOM_H
#define EXS_ATOM_H

#include <cmath>
#include <math.h>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace exs {

  template <typename T>
  class AtomBase {
  public:
    T value;
    // Copy constructor
    AtomBase(const AtomBase& a) {
      if constexpr (std::is_copy_constructible_v<T>) {
        value = a.value;
      } else {
        static_assert(std::is_copy_constructible_v<T>, "T is not copyable");
      }
    }
    // Move constructor
    AtomBase(AtomBase&& a) noexcept {
      if constexpr (std::is_move_constructible_v<T>) {
        value = std::move(a.value);
      } else {
        static_assert(std::is_move_constructible_v<T>, "T is not movable");
      }
    }
    // Constructor from value
    AtomBase(T v) : value(std::move(v)) {}
    // Copy assignment
    AtomBase& operator=(const AtomBase& a) {
      if constexpr (std::is_copy_assignable_v<T>) {
        value = a.value;
      } else {
        static_assert(std::is_copy_assignable_v<T>, "T is not copy-assignable");
      }
      return *this;
    }
    // Move assignment
    AtomBase& operator=(AtomBase&& a) noexcept {
      if constexpr (std::is_move_assignable_v<T>) {
        value = std::move(a.value);
      } else {
        static_assert(std::is_move_assignable_v<T>, "T is not move-assignable");
      }
      return *this;
    }
    virtual std::string to_string() = 0;
    void print() {
      std::cout << to_string() << "\n";
    }
  };

  typedef std::variant<float, bool> AtomValueType;

  class Atom : public AtomBase<AtomValueType> {
  public:
    Atom(Atom& a) : AtomBase(a) {};
    Atom(AtomValueType v) : AtomBase(v) {};
    static AtomValueType from_string(std::string s) {
      // NOTE: In case Settings are passed to the Solver, this method needs to be defined as
      //       from_string(std::string s, Settings* set = nullptr)
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
    std::string to_string() {
      if (std::holds_alternative<float>(value)) {
        std::stringstream str;
        str << std::get<float>(value) << std::scientific;
        return str.str();
      } else {
        if (std::get<bool>(value) == 0)
          return "false";
        else
          return "true";
      }
    }
    // Math operations
    void math_add(Atom* other) {
      value = std::get<float>(value) + std::get<float>(other->value);
    }
    void math_subtract(Atom* other) {
      value = std::get<float>(value) - std::get<float>(other->value);
    }
    void math_negate() {
      value = -std::get<float>(value);
    }
    void math_multiply(Atom* other) {
      value = std::get<float>(value) * std::get<float>(other->value);
    }
    void math_divide(Atom* other) {
      value = std::get<float>(value) / std::get<float>(other->value);
    }
    void math_power(Atom* other) {
      value = pow(std::get<float>(value), std::get<float>(other->value));
    }
    void math_modulo(Atom* other) {
      value = (float)((int)std::get<float>(value) % (int)std::get<float>(other->value));
    }

    // Argument math operators
    void math_exponent() {
      value = std::exp(std::get<float>(value));
    }
    void math_logarithm() {
      value = std::log(std::get<float>(value));
    }
    void math_logarithm_10() {
      value = std::log10(std::get<float>(value));
    }
    void math_logarithm_base(Atom* other) {
      value = std::log(std::get<float>(value)) / std::log(std::get<float>(other->value));
    }
    void math_power_base(Atom* other) {
      value = pow(std::get<float>(value), std::get<float>(other->value));
    }
    void math_square_root() {
      value = std::sqrt(std::get<float>(value));
    }
    void math_sinus() {
      value = std::sin(std::get<float>(value));
    }
    void math_cosinus() {
      value = std::cos(std::get<float>(value));
    }
    void math_tangens() {
      value = std::tan(std::get<float>(value));
    }

    // Comparison operations
    void comparison_equal(Atom* other) {
      value = std::get<float>(value) == std::get<float>(other->value);
    }
    void comparison_not_equal(Atom* other) {
      value = std::get<float>(value) != std::get<float>(other->value);
    }
    void comparison_less_equal(Atom* other) {
      value = std::get<float>(value) <= std::get<float>(other->value);
    }
    void comparison_greater_equal(Atom* other) {
      value = std::get<float>(value) >= std::get<float>(other->value);
    }
    void comparison_less(Atom* other) {
      value = std::get<float>(value) < std::get<float>(other->value);
    }
    void comparison_greater(Atom* other) {
      value = std::get<float>(value) > std::get<float>(other->value);
    }

    // Logical operations
    void logical_not() {
      value = !std::get<bool>(value);
    }
    void logical_and(Atom* other) {
      value = std::get<bool>(value) && std::get<bool>(other->value);
    }
    void logical_or(Atom* other) {
      value = std::get<bool>(value) || std::get<bool>(other->value);
    }

    // Conditional operator
    void condition(Atom* option1, Atom* option2) {
      value = std::get<bool>(value) ? std::get<float>(option1->value) : std::get<float>(option2->value);
    }
  };

} // namespace exs

#endif // EXS_ATOM_H
