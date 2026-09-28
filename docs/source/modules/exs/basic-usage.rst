Using the EXS solver
====================

``snt::exs::Solver`` evaluates an expression into an atom. The default
``snt::exs::Atom`` handles numeric and Boolean values, and the default solver
registers arithmetic, comparison, logical, grouping, and mathematical
operators with an evaluation order.

.. code-block:: cpp

   #include <snt/exs/solver.h>

   snt::exs::Solver<snt::exs::Atom> solver;
   auto result = solver.eval("23 * 34.5 + 4");
   result.print();  // 797.5

The ``eval`` call tokenizes the input, converts operands through
``Atom::from_string``, applies the configured operations, and returns the final
atom. Constructing a solver without arguments uses its built-in operator
registry and evaluation steps. Link an application that uses this API against
``snt-exs``.

For a runnable project, see the :doc:`DefaultSolver example
<../../examples/exs>`. To change which operators are available or how they
are ordered, continue with :doc:`Operators and evaluation order
<operators-and-order>`.
