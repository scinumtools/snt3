#ifndef PUQ_CALC_H
#define PUQ_CALC_H

#include "../quantity.h"
#include "../settings.h"
#include "../systems/systems.h"

#include <cmath>
#include <set>

namespace puq {

  class CalculatorExcept : public std::exception {
  private:
    std::string message;

  public:
    CalculatorExcept(const std::string& m) : message(m) {}
    const char* what() const noexcept override {
      return message.c_str();
    }
  };

  class CalculatorAtom : public exs::AtomBase<Quantity> {
  public:
    CalculatorAtom(CalculatorAtom& a) : AtomBase(a) {};
    CalculatorAtom(Quantity v) : AtomBase(v) {};
    static Quantity from_string(std::string& s);
    std::string to_string();
    void math_add(CalculatorAtom* other);
    void math_subtract(CalculatorAtom* other);
    void math_negate();
    void math_multiply(CalculatorAtom* other);
    void math_divide(CalculatorAtom* other);
  };

  class Calculator {
  public:
    std::unique_ptr<exs::Solver<CalculatorAtom>> solver;
    Calculator();
    CalculatorAtom solve(const std::string& expression);
  };

} // namespace puq

#endif // PUQ_CALC_H
