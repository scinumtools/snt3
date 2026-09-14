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

Python API
----------

The Python API is generated from the installed ``scinumtools3`` binding. It
documents the public module objects and functions exposed by the compiled
extension, including the ``core``, ``val``, ``puq``, ``dip``, and ``api``
submodules when they are enabled in the build.

.. toctree::
   :maxdepth: 2

   python_core
   python_val
   python_puq
   python_dip
   python_api

C++ implementation details will be added to the C++ API documentation in a
future version.
