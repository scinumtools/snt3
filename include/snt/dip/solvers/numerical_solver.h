#ifndef DIP_NUMERICAL_SOLVER_H
#define DIP_NUMERICAL_SOLVER_H

#include <snt/dip/environment.h>
#include <snt/dip/solvers/numerical_atom.h>

namespace snt::dip {

  class NumericalSolver {
  public:
    std::unique_ptr<exs::Solver<NumericalAtom, NumericalSettings>> solver;
    NumericalSolver(Environment& env, const std::string& units = "");
    NumericalAtom eval(const std::string& expression);
  };

} // namespace snt::dip

#endif // DIP_NUMERICAL_SOLVER_H
