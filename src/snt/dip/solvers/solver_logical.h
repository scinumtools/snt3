#ifndef DIP_SOLVER_LOGICAL_H
#define DIP_SOLVER_LOGICAL_H

#include "../../exs.h"
#include "../../val.h"

namespace snt::dip {

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
  
}

#endif // DIP_SOLVER_LOGICAL_H
