#ifndef EXS_ATOM_BASE_H
#define EXS_ATOM_BASE_H

#include <snt/exs/atom_grand.h>

#include <cmath>
#include <iostream>
#include <math.h>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <variant>

namespace snt::exs {

  /**
   * @class AtomBase
   * @brief Base atom class
   *
   * @tparam ATOM Atom type class
   * @tparam VALUE Atom value type
   */
  template <typename ATOM, typename VALUE>
  class AtomBase: public AtomGrand {
  public:
    VALUE value;
    // Copy constructor
    AtomBase(const AtomBase& a) {
      if constexpr (std::is_copy_constructible_v<VALUE>) {
        value = a.value;
      } else {
        static_assert(std::is_copy_constructible_v<VALUE>, "Atom is not copyable");
      }
    }
    // Move constructor
    AtomBase(AtomBase&& a) noexcept {
      if constexpr (std::is_move_constructible_v<VALUE>) {
        value = std::move(a.value);
      } else {
        static_assert(std::is_move_constructible_v<VALUE>, "Atom is not movable");
      }
    }
    // Constructor from value
    AtomBase(VALUE v) : value(std::move(v)) {}
    // Copy assignment
    AtomBase& operator=(const AtomBase& a) {
      if constexpr (std::is_copy_assignable_v<VALUE>) {
        value = a.value;
      } else {
        static_assert(std::is_copy_assignable_v<VALUE>, "Atom is not copy-assignable");
      }
      return *this;
    }
    // Move assignment
    AtomBase& operator=(AtomBase&& a) noexcept {
      if constexpr (std::is_move_assignable_v<VALUE>) {
        value = std::move(a.value);
      } else {
        static_assert(std::is_move_assignable_v<VALUE>, "Atom is not move-assignable");
      }
      return *this;
    }
    
    // Math operations
    virtual void math_add(ATOM* other) {
      throw std::runtime_error("Atom method 'math_add' is not implemented");
    };
    void math_add(AtomGrand* other) override {
      auto* casted = static_cast<ATOM*>(other);
      static_cast<ATOM*>(this)->math_add(casted);
    };
    virtual void math_subtract(ATOM* other) {
      throw std::runtime_error("Atom method 'math_subtract' is not implemented");
    };
    void math_subtract(AtomGrand* other) override {
      auto* casted = static_cast<ATOM*>(other);
      static_cast<ATOM*>(this)->math_subtract(casted);
    };
    virtual void math_multiply(ATOM* other) {
      throw std::runtime_error("Atom method 'math_multiply' is not implemented");
    };
    void math_multiply(AtomGrand* other) override {
      auto* casted = static_cast<ATOM*>(other);
      static_cast<ATOM*>(this)->math_multiply(casted);
    };
    virtual void math_divide(ATOM* other) {
      throw std::runtime_error("Atom method 'math_divide' is not implemented");
    };
    void math_divide(AtomGrand* other) override {
      auto* casted = static_cast<ATOM*>(other);
      static_cast<ATOM*>(this)->math_divide(casted);
    };
    virtual void math_power(ATOM* other) {
      throw std::runtime_error("Atom method 'math_power' is not implemented");
    };
    void math_power(AtomGrand* other) override {
      auto* casted = static_cast<ATOM*>(other);
      static_cast<ATOM*>(this)->math_power(casted);
    };
    virtual void math_modulo(ATOM* other) {
      throw std::runtime_error("Atom method 'math_modulo' is not implemented");
    };
    void math_modulo(AtomGrand* other) override {
      auto* casted = static_cast<ATOM*>(other);
      static_cast<ATOM*>(this)->math_modulo(casted);
    };

    // Argument math operators
    virtual void math_logarithm_base(ATOM* other) {
      throw std::runtime_error("Atom method 'math_logarithm_base' is not implemented");
    };
    void math_logarithm_base(AtomGrand* other) override {
      auto* casted = static_cast<ATOM*>(other);
      static_cast<ATOM*>(this)->math_logarithm_base(casted);
    };
    virtual void math_power_base(ATOM* other) {
      throw std::runtime_error("Atom method 'math_power_base' is not implemented");
    };
    void math_power_base(AtomGrand* other) override {
      auto* casted = static_cast<ATOM*>(other);
      static_cast<ATOM*>(this)->math_power_base(casted);
    };

    // Comparison operations
    virtual void comparison_equal(ATOM* other) {
      throw std::runtime_error("Atom method 'comparison_equal' is not implemented");
    };
    void comparison_equal(AtomGrand* other) override {
      auto* casted = static_cast<ATOM*>(other);
      static_cast<ATOM*>(this)->comparison_equal(casted);
    };
    virtual void comparison_not_equal(ATOM* other) {
      throw std::runtime_error("Atom method 'comparison_not_equal' is not implemented");
    };
    void comparison_not_equal(AtomGrand* other) override {
      auto* casted = static_cast<ATOM*>(other);
      static_cast<ATOM*>(this)->comparison_not_equal(casted);
    };
    virtual void comparison_less_equal(ATOM* other) {
      throw std::runtime_error("Atom method 'comparison_less_equal' is not implemented");
    };
    void comparison_less_equal(AtomGrand* other) override {
      auto* casted = static_cast<ATOM*>(other);
      static_cast<ATOM*>(this)->comparison_less_equal(casted);
    };
    virtual void comparison_greater_equal(ATOM* other) {
      throw std::runtime_error("Atom method 'comparison_greater_equal' is not implemented");
    };
    void comparison_greater_equal(AtomGrand* other) override {
      auto* casted = static_cast<ATOM*>(other);
      static_cast<ATOM*>(this)->comparison_greater_equal(casted);
    };
    virtual void comparison_less(ATOM* other) {
      throw std::runtime_error("Atom method 'comparison_less' is not implemented");
    };
    void comparison_less(AtomGrand* other) override {
      auto* casted = static_cast<ATOM*>(other);
      static_cast<ATOM*>(this)->comparison_less(casted);
    };
    virtual void comparison_greater(ATOM* other) {
      throw std::runtime_error("Atom method 'comparison_greater' is not implemented");
    };
    void comparison_greater(AtomGrand* other) override {
      auto* casted = static_cast<ATOM*>(other);
      static_cast<ATOM*>(this)->comparison_greater(casted);
    };

    // Logical operations
    virtual void logical_and(ATOM* other) {
      throw std::runtime_error("Atom method 'logical_and' is not implemented");
    };
    void logical_and(AtomGrand* other) override {
      auto* casted = static_cast<ATOM*>(other);
      static_cast<ATOM*>(this)->logical_and(casted);
    };
    virtual void logical_or(ATOM* other) {
      throw std::runtime_error("Atom method 'logical_or' is not implemented");
    };
    void logical_or(AtomGrand* other) override {
      auto* casted = static_cast<ATOM*>(other);
      static_cast<ATOM*>(this)->logical_or(casted);
    };

    // Conditional operator
    virtual void condition(ATOM* option1, ATOM* option2) {
      throw std::runtime_error("Atom method 'condition' is not implemented");
    };
    void condition(AtomGrand* option1, AtomGrand* option2) override {
      auto* casted1 = static_cast<ATOM*>(option1);
      auto* casted2 = static_cast<ATOM*>(option2);
      static_cast<ATOM*>(this)->condition(casted1, casted2);
    };
  };

}
  
#endif // EXS_ATOM_BASE_H
