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

Optional expression structure
-----------------------------

Pass a ``CompositionGraph`` to ``eval`` when a caller also needs the structure
of an expression. The return value remains the evaluated atom; the graph is
filled through the optional argument.
The graph contains operand, operator, and group nodes. Each node's
``children`` are node indices in source order; ``root`` identifies the final
expression. Operand ``text`` contains the original operand token, while
operator and group ``text`` contains the registered operator name.

.. code-block:: cpp

   snt::exs::CompositionGraph graph;
   auto result = solver.eval("1 + 2 * 3", &graph);  // result is 7
   const auto& root = graph.nodes.at(graph.root);  // "add"
   const auto& right = graph.nodes.at(root.children.at(1));  // "mul"

The expression above becomes this nested structure::

   add
   ├── 1
   └── mul
       ├── 2
       └── 3

Thus ``add`` combines the operand ``1`` with the result of ``mul(2, 3)``.
The edges show which items each operation acts on, as well as how the
operations nest.

``eval`` with a graph argument records the operations performed by the same
tokenization and evaluation path used by ``eval``; it does not parse the
expression a second time. The standalone ``build_composition_graph`` shares
the lexical scan and configured reduction steps, but does not evaluate atoms.

The graph describes syntax, including both arms of a conditional expression.
It does not identify DIP references, record which branch was selected, or
store evaluated intermediate values. Callers can interpret operand text in
their own domain. Ordinary ``eval`` does not build a graph.

For a runnable project, see the :doc:`DefaultSolver example
<../../examples/exs>`. To change which operators are available or how they
are ordered, continue with :doc:`Operators and evaluation order
<operators-and-order>`.
