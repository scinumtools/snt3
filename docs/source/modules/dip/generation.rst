Static Parameter Generation
===========================

An evaluated ``dip::Environment`` can be exported as static, language-native
parameters. This is intended for simulations and compiled applications that
need to include a validated DIPL configuration directly, without parsing DIPL
at run time. Supported formats are C++, C, Fortran, Rust, Julia, JSON, and
YAML.

Native representation
---------------------

Groups become nested native structures. Lists retain their order, maps retain
their keys as iterable key/value entries, and scalar arrays are emitted as
native nested collections. C++ uses ``std::array`` and supports one-, two-,
and three-dimensional arrays without an additional matrix-library dependency.
The Julia exporter uses immutable named tuples, tuples, and ``key => value``
pairs for the same purpose.

C++
---

From C++:

.. code-block:: cpp

   environment.generate(snt::dip::ExportFormat::CPP, "parameters.hpp");
   environment.generate(snt::dip::ExportFormat::JULIA, "parameters.jl");

Generation is also available from the :doc:`Python binding
<../../integrations/python>`, the :doc:`C binding <../../integrations/c>`,
the :doc:`command-line interface <../../integrations/cli>`, and the
:doc:`command-oriented C++ API <../../api/dip>`.

Command line
------------

For command-line workflows, parse a DIPL input or load a DIPH5 environment
and select an output format:

.. code-block:: console

   snt dip parse --input file config.dip --generate cpp parameters.hpp
   snt dip parse --load config.diph5 --generate julia parameters.jl

The CLI format names are ``cpp``, ``c``, ``fortran``, ``rust``, ``julia``,
``json``, and ``yaml``. Generation exports the complete evaluated environment;
request and tag filters only control text printed by the command.
