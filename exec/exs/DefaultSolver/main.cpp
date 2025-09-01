#include "exs.h"

using namespace snt::exs;

int main() {

  // using default operations and operation steps
  Solver<Atom> solver;
  Atom atom = solver.solve("23 * 34.5 + 4");
  atom.print();

  /*
   * Equivalent c++ notation:
   * (23 * 34.5
   ) + 4 == 797.5
   */
}
