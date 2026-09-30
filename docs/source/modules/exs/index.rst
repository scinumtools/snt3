.. _exs-expression-solver:

EXS — Expression Solver
-----------------------

The ``EXS`` (Expression Solver) module provides the general parsing and
evaluation infrastructure for expressions in SciNumTools. It is designed
as a flexible foundation on which specialized expression solvers can be
built by defining the available operations and their evaluation order.

This makes it possible to create domain-specific expression languages
without implementing a separate parser and evaluation engine for each
application. Operations may represent numerical, logical, mathematical,
or domain-specific functionality, while EXS takes care of their
interpretation and evaluation.

PUQ and DIP build their specialized solvers on EXS. Its configurable parser
and evaluation steps let those modules share expression infrastructure.

* :doc:`Using the EXS solver <basic-usage>` — evaluate an expression with
  the built-in atom and operators.
* :doc:`Operators and evaluation order <operators-and-order>` — choose
  operator symbols and specify the order in which operations run.
* :doc:`Custom atoms and operations <custom-operations>` — add a new
  operator, its evaluation step, and application-specific atom behavior.
* :doc:`Solver settings <settings>` — pass application data to atom
  parsing and operator evaluation.
* :doc:`EXS examples <../../examples/exs>` — runnable default, modified, and
  custom solvers.

.. toctree::
   :maxdepth: 1
   :hidden:

   basic-usage
   operators-and-order
   custom-operations
   settings

