EXS examples
============

The ``examples/exs`` directory contains small, focused C++ projects showing
how to extend and configure the EXS expression solver.

DefaultSolver
-------------

The `DefaultSolver source <https://github.com/vrtulka23/scinumtools3/tree/main/examples/exs/DefaultSolver>`_ evaluates a basic expression:

It uses the standard ``Atom``, operator registry, and evaluation steps. This
is the smallest starting point for embedding EXS in a C++ program and shows
the default arithmetic precedence.

.. code-block:: cpp

   snt::exs::Solver<snt::exs::Atom> solver;
   auto result = solver.eval("23 * 34.5 + 4");

ModifiedSolver
--------------

The `ModifiedSolver source <https://github.com/vrtulka23/scinumtools3/tree/main/examples/exs/ModifiedSolver>`_ changes logical symbols and precedence:

The example registers ``N``, ``A``, and ``O`` for negation, conjunction, and
disjunction, then supplies an explicit step order. It demonstrates that
syntax and precedence are configuration choices rather than fixed parser
rules.

.. code-block:: text

   N false A false O true

CustomSolver
------------

The `CustomSolver source <https://github.com/vrtulka23/scinumtools3/tree/main/examples/exs/CustomSolver>`_ adds a custom ``len(...)`` operator:

Its atom stores either strings, booleans, or lengths. A custom group operator
converts the argument to a length before the comparison operator runs, showing
how domain-specific operations can be introduced without changing EXS.

.. code-block:: text

   apple < len(hospital)

ArraySolver
-----------

The `ArraySolver source <https://github.com/vrtulka23/scinumtools3/tree/main/examples/exs/ArraySolver>`_ adds array syntax and element-wise operations:

The custom atom stores ``std::vector<double>`` values, while a group operator
parses brackets and commas. The example then implements vector addition and
subtraction through the normal EXS operation pipeline.

.. code-block:: text

   [1, 2, 3] + [4, 5, 6]

SettingsSolver
--------------

The `SettingsSolver source <https://github.com/vrtulka23/scinumtools3/tree/main/examples/exs/SettingsSolver>`_ uses application settings in expressions:

An application-defined settings object is passed to the solver. Custom
operators use it to substitute ``?`` and select an indexed option, showing how
evaluation can depend on host application state.

.. code-block:: text

   2 + ? - {3}

UniquePtrSolver
---------------

The `UniquePtrSolver source <https://github.com/vrtulka23/scinumtools3/tree/main/examples/exs/UniquePtrSolver>`_ evaluates a logical expression with an atom backed by ``std::unique_ptr``:

The custom atom implements deep-copy construction and assignment in addition
to move operations. This demonstrates that EXS atoms can own non-copyable
resources while still satisfying the solver's value semantics.

.. code-block:: text

   true && false
