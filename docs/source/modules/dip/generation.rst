Static parameter generation from C++
====================================

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

Parse a DIPfile project, then generate output from the evaluated environment:

.. code-block:: cpp

   #include <snt/dip/dip.h>

   snt::dip::DIP parser;
   parser.add_project("DIPfile");
   auto environment = parser.parse();

   environment.generate(snt::dip::ExportFormat::CPP, "parameters.hpp");
   environment.generate(snt::dip::ExportFormat::JULIA, "parameters.jl");

The C++ ``ExportFormat`` values implemented by the generator are ``CPP``,
``C``, ``FORTRAN``, ``RUST``, ``JULIA``, ``JSON``, and ``YAML``. Each call
exports the complete evaluated environment. For the command-oriented C++
interface, see :ref:`the DIP API guide <module-api-dip>`.
