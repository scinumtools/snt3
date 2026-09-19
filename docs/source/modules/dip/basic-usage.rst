Basic Usage
===========

The usual C++ DIP workflow is to create a ``DIP`` parser, add one or more DIPL
inputs, and call ``parse()``. Parsing evaluates the definitions and returns an
``Environment`` containing the resulting value nodes.

The file-based calls below assume two separate files in the program's current
working directory. Relative filenames are resolved from that directory.

``parameters.dipl``:

.. code-block:: dipl

   width float = 40 cm

``shared.dipl``:

.. code-block:: dipl

   scale float = 2

Parsing inputs
--------------

.. code-block:: cpp

   snt::dip::DIP dip;
   dip.add_string("length float = 2.5 m");
   dip.add_file("parameters.dipl");
   dip.add_unit("length_unit", "23*au");
   dip.add_source("shared", "shared.dipl");

   snt::dip::Environment env = dip.parse();
   double length = env["length"].as<double>();
   env.save("parameters.diph5");

``add_string`` and ``add_file`` queue definitions for evaluation by
``parse()``. In contrast, ``add_source(name, file)`` reads and evaluates an
external DIPL file immediately in a separate source environment. Its values
can then be referenced using expressions such as ``{shared?scale}``; they
are not automatically added to the local node tree. Register custom units
with ``add_unit(name, expression)`` before parsing the local definitions.

Environment
-----------

An environment can be queried by path using ``env["path.to.value"]``. The
resulting cursor exposes the evaluated value and its units; groups, maps, and
lists can be traversed when a definition contains collections. Use
``env.load("parameters.diph5")`` to restore a previously persisted evaluated environment
without parsing the original DIPL files.

For custom behavior, DIP also supports registered value- and node-producing
functions. Those functions are evaluated while parsing and are not part of a
persisted DIPH5 environment; applications that load DIPH5 should treat the
file as evaluated data. The complete class and function reference is available
in the :doc:`C++ DIP API <../../api/cpp_dip>` documentation.

For Python, C, command-line, and CMake usage, see
:doc:`Interfaces and integrations <../../integrations/index>`.
