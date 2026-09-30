Programming Reference
=====================

This reference documents the C++, Python, C, and CMake interfaces. The
``snt::api`` namespace is one of the C++ modules documented here.

C++ API
-------

The C++ reference covers the public interfaces of each SciNumTools
module. These interfaces are intended for users developing scientific
applications and provide access to values, expressions, physical
quantities, and dimensional input parameters.

The ``API`` module supplies command-oriented application operations such as
PUQ evaluation and DIPL parsing. Its declarations appear alongside those of
the other C++ modules.

Each module overview links to topic pages for related types and operations.
For examples and workflow guidance, see the :ref:`API module overview
<api-application-interface>`.

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
The :ref:`Python adapter reference <python-dip-adapters>` lists the matching
classes and runner functions.

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

C API
-----

The C ABI reference documents the experimental opaque-handle interfaces for
PUQ quantities and DIPL parsing. The adapter interface is provided through
the C++ and Python APIs linked above; it has no C ABI entry points.

.. toctree::
   :maxdepth: 1

   c_puq
   c_dip

Reference scope
---------------

The C++ reference is generated from declarations and comments using Doxygen
and Breathe. It covers the namespaces in ``include/snt/`` and the
``snt::api`` command headers in ``src/snt/api``, including public members,
free functions, enums, and aliases. Private members, the ``dip::detail``
template helpers, and the recursive ``core::_array_to_string`` helpers are
omitted. The VAL page also omits the string-array template specialization
to avoid duplicate documentation links. MAT is included as an experimental
module.

The :doc:`C++ adapter reference <cpp/dip/adapter>` lists ``Adapter``,
``AdapterContext``, and the three ``run_adapter*`` functions.
