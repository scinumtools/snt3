#ifndef PUQ_UNIT_SOLVER_H
#define PUQ_UNIT_SOLVER_H

#include "../../exs/pch_exs.h"
#include "unit_atom.h"

namespace snt::puq {

  class UnitSolver {
  public:
    std::unique_ptr<exs::Solver<UnitAtom>> solver;
    UnitSolver();
    UnitAtom solve(const std::string& expression);
  };

} // namespace snt::puq

#endif // PUQ_UNIT_SOLVER_H
