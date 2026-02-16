#ifndef DIP_SOLVER_NUMERICAL_H
#define DIP_SOLVER_NUMERICAL_H

#include "../../exs.h"
#include "../../val.h"
#include "../environment.h"

namespace snt::dip {

  struct NumericalSettings: exs::BaseSettings {
    Environment* env;
  };

  class NumericalAtom : public exs::AtomBase<NumericalAtom,val::BaseValue::PointerType> {
  public:
    // Constructor from unique_ptr
    NumericalAtom(val::BaseValue::PointerType b) : AtomBase(std::move(b)) {};
    // Deep copy constructor
    NumericalAtom(const NumericalAtom& a);
    NumericalAtom& operator=(const NumericalAtom& a);
    // Move constructor
    NumericalAtom(NumericalAtom&& a) noexcept = default;
    NumericalAtom& operator=(NumericalAtom&& a) noexcept = default;

    static val::BaseValue::PointerType from_string(const std::string& s, exs::BaseSettings* settings);
    std::string to_string() override;
    void math_sinus() override;
    void math_cosinus() override;
    void math_tangens() override;
    void math_cubic_root() override;
    void math_square_root() override;
    // void math_power_base(NumericalAtom* other) override;
    // void math_logarithm_base(NumericalAtom* other) override;
    void math_logarithm_10() override;
    void math_logarithm() override;
    void math_exponent() override;
    void math_power(NumericalAtom* other) override;
    void math_multiply(NumericalAtom* other) override;
    void math_divide(NumericalAtom* other) override;
    void math_add(NumericalAtom* other) override;
    void math_subtract(NumericalAtom* other) override;
    void math_negate() override;
  };

  class NumericalSolver {
  public:
    std::unique_ptr<exs::Solver<NumericalAtom,NumericalSettings>> solver;
    NumericalSolver(Environment& env);
    NumericalAtom eval(const std::string& expression);
  };

} // namespace snt::dip

#endif // DIP_SOLVER_NUMERICAL_H
