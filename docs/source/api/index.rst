API Reference
=============

The code API is documented through two language interfaces: C++ and Python.
Both expose the same scientific concepts; the ``snt::api`` namespace is a
regular C++ module within the C++ interface.

C++ API
-------

The module API provides the main public interfaces of each SciNumTools
module. These interfaces are intended for users developing scientific
applications and provide access to values, expressions, physical
quantities, and dimensional input parameters.

The ``API`` module supplies command-oriented application operations such as
PUQ evaluation and DIPL parsing. It is documented as part of the C++ module
API, rather than as a separate code API.

The reference below is generated from C++ declarations and comments using
Doxygen and Breathe. It covers the namespaces in ``include/snt/`` and the
``snt::api`` command headers in ``src/snt/api``, including public members,
free functions, enums, and aliases. Private members, the ``dip::detail``
template helpers, and the recursive ``core::_array_to_string`` helpers are
omitted. The VAL page also omits the string-array template specialization
to avoid duplicate documentation links. MAT is included as an experimental
module.

Each module overview links to topic pages that group related types and
operations, such as quantities, unit systems, or DIPL nodes.

.. toctree::
   :maxdepth: 1

   cpp_core
   cpp_val
   cpp_exs
   cpp_puq
   cpp_dip
   cpp_mat
   cpp_api

Python API
----------

The Python API is generated from the installed ``scinumtools3`` binding. It
documents the public module objects and functions exposed by the compiled
extension, including the ``core``, ``val``, ``puq``, ``dip``, and ``api``
submodules when they are enabled in the build. EXS is used internally by
PUQ and DIP and is not exposed as a standalone Python module; see
:doc:`../integrations/python` for the rationale.

.. toctree::
   :maxdepth: 2

   python_core
   python_val
   python_puq
   python_dip
   python_api

CMake API
---------

The CMake integration exposes the installed ``snt`` executable and helper
functions for evaluating DIPL configuration during project configuration.

.. toctree::
   :maxdepth: 1

   cmake
