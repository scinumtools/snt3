#ifndef EXS_ATOM_H
#define EXS_ATOM_H

#include "settings.h"

#include <cmath>
#include <iostream>
#include <math.h>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <variant>

namespace snt::exs {

  class AtomGrand {
  public:
    virtual ~AtomGrand() = default;
    
    virtual std::string to_string() = 0;
    void print() {
      std::cout << to_string() << "\n";
    }

    // Math operations
    virtual void math_add(AtomGrand* other) {};
    virtual void math_subtract(AtomGrand* other) {};
    virtual void math_negate() {};
    virtual void math_multiply(AtomGrand* other) {};
    virtual void math_divide(AtomGrand* other) {};
    virtual void math_power(AtomGrand* other) {};
    virtual void math_modulo(AtomGrand* other) {};

    // Argument math operators
    virtual void math_exponent() {};
    virtual void math_logarithm() {};
    virtual void math_logarithm_10() {};
    virtual void math_logarithm_base(AtomGrand* other) {};
    virtual void math_power_base(AtomGrand* other) {};
    virtual void math_square_root() {};
    virtual void math_cubic_root() {};
    virtual void math_sinus() {};
    virtual void math_cosinus() {};
    virtual void math_tangens() {};

    // Comparison operations
    virtual void comparison_equal(AtomGrand* other) {};
    virtual void comparison_not_equal(AtomGrand* other) {};
    virtual void comparison_less_equal(AtomGrand* other) {};
    virtual void comparison_greater_equal(AtomGrand* other) {};
    virtual void comparison_less(AtomGrand* other) {};
    virtual void comparison_greater(AtomGrand* other) {};

    // Logical operations
    virtual void logical_not() {};
    virtual void logical_and(AtomGrand* other) {};
    virtual void logical_or(AtomGrand* other) {};

    // Conditional operator
    virtual void condition(AtomGrand* option1, AtomGrand* option2) {};
  };
  
  template <typename A, typename T>
  class AtomBase: public AtomGrand {
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
    
    // Math operations
    virtual void math_add(A* other) {};
    void math_add(AtomGrand* other) override {
      auto* casted = static_cast<A*>(other);
      static_cast<A*>(this)->math_add(casted);
    };
    virtual void math_subtract(A* other) {};
    void math_subtract(AtomGrand* other) override {
      auto* casted = static_cast<A*>(other);
      static_cast<A*>(this)->math_subtract(casted);
    };
    virtual void math_multiply(A* other) {};
    void math_multiply(AtomGrand* other) override {
      auto* casted = static_cast<A*>(other);
      static_cast<A*>(this)->math_multiply(casted);
    };
    virtual void math_divide(A* other) {};
    void math_divide(AtomGrand* other) override {
      auto* casted = static_cast<A*>(other);
      static_cast<A*>(this)->math_divide(casted);
    };
    virtual void math_power(A* other) {};
    void math_power(AtomGrand* other) override {
      auto* casted = static_cast<A*>(other);
      static_cast<A*>(this)->math_power(casted);
    };
    virtual void math_modulo(A* other) {};
    void math_modulo(AtomGrand* other) override {
      auto* casted = static_cast<A*>(other);
      static_cast<A*>(this)->math_modulo(casted);
    };

    // Argument math operators
    virtual void math_logarithm_base(A* other) {};
    void math_logarithm_base(AtomGrand* other) override {
      auto* casted = static_cast<A*>(other);
      static_cast<A*>(this)->math_logarithm_base(casted);
    };
    virtual void math_power_base(A* other) {};
    void math_power_base(AtomGrand* other) override {
      auto* casted = static_cast<A*>(other);
      static_cast<A*>(this)->math_power_base(casted);
    };

    // Comparison operations
    virtual void comparison_equal(A* other) {};
    void comparison_equal(AtomGrand* other) override {
      auto* casted = static_cast<A*>(other);
      static_cast<A*>(this)->comparison_equal(casted);
    };
    virtual void comparison_not_equal(A* other) {};
    void comparison_not_equal(AtomGrand* other) override {
      auto* casted = static_cast<A*>(other);
      static_cast<A*>(this)->comparison_not_equal(casted);
    };
    virtual void comparison_less_equal(A* other) {};
    void comparison_less_equal(AtomGrand* other) override {
      auto* casted = static_cast<A*>(other);
      static_cast<A*>(this)->comparison_less_equal(casted);
    };
    virtual void comparison_greater_equal(A* other) {};
    void comparison_greater_equal(AtomGrand* other) override {
      auto* casted = static_cast<A*>(other);
      static_cast<A*>(this)->comparison_greater_equal(casted);
    };
    virtual void comparison_less(A* other) {};
    void comparison_less(AtomGrand* other) override {
      auto* casted = static_cast<A*>(other);
      static_cast<A*>(this)->comparison_less(casted);
    };
    virtual void comparison_greater(A* other) {};
    void comparison_greater(AtomGrand* other) override {
      auto* casted = static_cast<A*>(other);
      static_cast<A*>(this)->comparison_greater(casted);
    };

    // Logical operations
    virtual void logical_and(A* other) {};
    void logical_and(AtomGrand* other) override {
      auto* casted = static_cast<A*>(other);
      static_cast<A*>(this)->logical_and(casted);
    };
    virtual void logical_or(A* other) {};
    void logical_or(AtomGrand* other) override {
      auto* casted = static_cast<A*>(other);
      static_cast<A*>(this)->logical_or(casted);
    };

    // Conditional operator
    virtual void condition(A* option1, A* option2) {};
    void condition(AtomGrand* option1, AtomGrand* option2) override {
      auto* casted1 = static_cast<A*>(option1);
      auto* casted2 = static_cast<A*>(option2);
      static_cast<A*>(this)->condition(casted1, casted2);
    };
  };

  typedef std::variant<double, bool> AtomValueType;

  class Atom final: public AtomBase<Atom,AtomValueType> {
  public:
    
    Atom(Atom& a) : AtomBase(a) {};
    
    Atom(AtomValueType v) : AtomBase(v) {};
    
    static AtomValueType from_string(std::string s, EmptySettings* set = nullptr) {
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
    
    std::string to_string() override {
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
    void math_add(Atom* other) override {
      value = std::get<double>(value) + std::get<double>(other->value);
    }
    void math_subtract(Atom* other) override {
      value = std::get<double>(value) - std::get<double>(other->value);
    }
    void math_negate() override {
      value = -std::get<double>(value);
    }
    void math_multiply(Atom* other) override {
      value = std::get<double>(value) * std::get<double>(other->value);
    }
    void math_divide(Atom* other) override {
      value = std::get<double>(value) / std::get<double>(other->value);
    }
    void math_power(Atom* other) override {
      value = pow(std::get<double>(value), std::get<double>(other->value));
    }
    void math_modulo(Atom* other) override {
      value = (double)((int)std::get<double>(value) % (int)std::get<double>(other->value));
    }

    // Argument math operators
    void math_exponent() override {
      value = std::exp(std::get<double>(value));
    }
    void math_logarithm() override {
      value = std::log(std::get<double>(value));
    }
    void math_logarithm_10() override {
      value = std::log10(std::get<double>(value));
    }
    void math_logarithm_base(Atom* other) override {
      value = std::log(std::get<double>(value)) / std::log(std::get<double>(other->value));
    }
    void math_power_base(Atom* other) override {
      value = pow(std::get<double>(value), std::get<double>(other->value));
    }
    void math_square_root() override {
      value = std::sqrt(std::get<double>(value));
    }
    void math_cubic_root() override {
      value = std::cbrt(std::get<double>(value));
    }
    void math_sinus() override {
      value = std::sin(std::get<double>(value));
    }
    void math_cosinus() override {
      value = std::cos(std::get<double>(value));
    }
    void math_tangens() override {
      value = std::tan(std::get<double>(value));
    }

    // Comparison operations
    void comparison_equal(Atom* other) override {
      value = std::get<double>(value) == std::get<double>(other->value);
    }
    void comparison_not_equal(Atom* other) override {
      value = std::get<double>(value) != std::get<double>(other->value);
    }
    void comparison_less_equal(Atom* other) override {
      value = std::get<double>(value) <= std::get<double>(other->value);
    }
    void comparison_greater_equal(Atom* other) override {
      value = std::get<double>(value) >= std::get<double>(other->value);
    }
    void comparison_less(Atom* other) override {
      value = std::get<double>(value) < std::get<double>(other->value);
    }
    void comparison_greater(Atom* other) override {
      value = std::get<double>(value) > std::get<double>(other->value);
    }

    // Logical operations
    void logical_not() override {
      value = !std::get<bool>(value);
    }
    void logical_and(Atom* other) override {
      value = std::get<bool>(value) && std::get<bool>(other->value);
    }
    void logical_or(Atom* other) override {
      value = std::get<bool>(value) || std::get<bool>(other->value);
    }

    // Conditional operator
    void condition(Atom* option1, Atom* option2) override {
      value = std::get<bool>(value) ? std::get<double>(option1->value) : std::get<double>(option2->value);
    }
  };

} // namespace snt::exs

#endif // EXS_ATOM_H
