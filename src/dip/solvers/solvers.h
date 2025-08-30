#ifndef H_SOLVERS
#define H_SOLVERS

#include "../../val/values_array.h"
#include "../environment.h"

namespace dip {

  // Logical solver
  /////////////////

  struct LogicalSettings {
    Environment* env;
  };

  class LogicalAtom : public exs::AtomBase<val::BaseValue::PointerType> {
  public:
    // Constructor from unique_ptr
    LogicalAtom(val::BaseValue::PointerType b) : AtomBase(std::move(b)) {};
    // Deep copy constructor
    LogicalAtom(const LogicalAtom& a);
    LogicalAtom& operator=(const LogicalAtom& a);
    // Move constructor
    LogicalAtom(LogicalAtom&& a) noexcept = default;
    LogicalAtom& operator=(LogicalAtom&& a) noexcept = default;

    static val::BaseValue::PointerType from_string(const std::string& s, LogicalSettings* settings);
    std::string to_string();
    void comparison_equal(LogicalAtom* other);
    void comparison_not_equal(LogicalAtom* other);
    void comparison_less_equal(LogicalAtom* other);
    void comparison_greater_equal(LogicalAtom* other);
    void comparison_less(LogicalAtom* other);
    void comparison_greater(LogicalAtom* other);
    void logical_not();
    void logical_and(LogicalAtom* other);
    void logical_or(LogicalAtom* other);
  };

  class LogicalSolver {
  public:
    std::unique_ptr<exs::Solver<LogicalAtom, LogicalSettings>> solver;
    LogicalSolver(Environment& env);
    LogicalAtom eval(const std::string& expression);
  };

  // Numerical solver
  ///////////////////

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

  // Template solver
  //////////////////

  class TemplateSolver {};

} // namespace dip

#endif // H_SOLVERS
