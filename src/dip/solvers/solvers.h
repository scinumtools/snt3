#ifndef H_SOLVERS
#define H_SOLVERS

#include "../../exs/exs.h"
#include "../../val/values.h"
#include "../environment.h"

namespace dip {

  // Logical solver
  
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
    //void comparison_equal(NumericalAtom* other);
    //void comparison_not_equal(NumericalAtom* other);
    //void comparison_less_equal(NumericalAtom* other);
    //void comparison_greater_equal(NumericalAtom* other);
    //void comparison_less(NumericalAtom* other);
    //void comparison_greater(NumericalAtom* other);
    //void numerical_not();
    //void numerical_and(NumericalAtom* other);
    //void numerical_or(NumericalAtom* other);
  };
  
  class NumericalSolver {
  public:
    std::unique_ptr<exs::Solver<NumericalAtom, NumericalSettings>> solver;
    NumericalSolver(Environment& env);
    NumericalAtom eval(const std::string& expression);
  };

  // Template solver
  
  class TemplateSolver {};

} // namespace dip

#endif // H_SOLVERS
