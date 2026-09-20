PUEL calculation
================

PUEL is the Physical Units Expression Language used by PUQ to describe
quantities, units, arithmetic, and supported mathematical operations. The
``Calculator`` evaluates a PUEL expression and returns its typed ``Quantity``
result.

Evaluating an expression
------------------------

Create a calculator when an application needs to evaluate user input or a
configuration expression at run time:

.. code-block:: cpp

   #include <snt/puq/calc/calculator.h>

   snt::puq::Calculator calculator;
   snt::puq::Quantity speed = calculator.eval("12*km / 3*h").value;

Expressions may combine compatible quantities. PUQ evaluates their numerical
operations and derives the output dimensions, while retaining any uncertainty
information carried by the inputs.

Parsing unit expressions
------------------------

When only a unit expression must be parsed or inspected, use the
``UnitSolver``. It is useful for validating a unit supplied independently of a
quantity value:

.. code-block:: cpp

   #include <snt/puq/solver/unit_solver.h>

   snt::puq::UnitSolver solver;
   auto unit = solver.eval("kg*m/s2");

The :doc:`PUEL specification <../../puel/index>` defines the language in
detail. For command-oriented C++ evaluation, formatting, and inspection, see
the :doc:`PUQ C++ API <../../api/puq>`.
