#ifndef DIP_SOLVER_NUMERICAL_H
#define DIP_SOLVER_NUMERICAL_H

#include "../../exs.h"
#include "../../val.h"

namespace snt::dip {

  struct NumericalSettings {
    Environment* env;
  };

  class NumericalAtom : public exs::AtomBase<val::BaseValue::PointerType> {
  public:
    // Constructor from unique_ptr
    NumericalAtom(val::BaseValue::PointerType b) : AtomBase(std::move(b)) {};
    // Deep copy constructor
    NumericalAtom(const NumericalAtom& a);
    NumericalAtom& operator=(const NumericalAtom& a);
    // Move constructor
    NumericalAtom(NumericalAtom&& a) noexcept = default;
    NumericalAtom& operator=(NumericalAtom&& a) noexcept = default;

    static val::BaseValue::PointerType from_string(const std::string& s, NumericalSettings* settings);
    std::string to_string();
    void math_sinus();
    void math_cosinus();
    void math_tangens();
    void math_cubic_root();
    void math_square_root();
    // void math_power_base(NumericalAtom* other);
    // void math_logarithm_base(NumericalAtom* other);
    void math_logarithm_10();
    void math_logarithm();
    void math_exponent();
    void math_power(NumericalAtom* other);
    void math_multiply(NumericalAtom* other);
    void math_divide(NumericalAtom* other);
    void math_add(NumericalAtom* other);
    void math_subtract(NumericalAtom* other);
    void math_negate();
  };

  class NumericalSolver {
  public:
    std::unique_ptr<exs::Solver<NumericalAtom, NumericalSettings>> solver;
    NumericalSolver(Environment& env);
    NumericalAtom eval(const std::string& expression);
  };

} // namespace snt::dip

#endif // DIP_SOLVER_NUMERICAL_H
