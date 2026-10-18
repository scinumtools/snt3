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
``C``, ``FORTRAN``, ``RUST``, ``JULIA``, ``JSON``, ``JSON_METADATA``, and
``YAML``. Each call exports the complete evaluated environment. The existing
``JSON`` format remains a values-only tree. ``JSON_METADATA`` writes a
versioned record for each evaluated value, keyed by its stable DIPL path, with
its type, shape, value, units, effective metadata, and declaration or override
provenance. Its ``node_metadata`` inventory is keyed by stable DIPL path and
includes effective metadata for metadata-bearing hierarchy nodes and values,
with each record's node kind. This covers groups, maps, lists, map/list items,
table headers, and table columns. Source identities and source-level metadata
remain in the separate ``sources`` object.
Array records retain explicit ``shape`` and ``is_array`` fields, so a scalar
string ``"example"`` remains distinct from a singleton array declared as
``["example"]``. The singleton's value payload may itself be the scalar
``"example"``; consumers should use the shape fields to reconstruct its array
classification. Nested arrays retain their JSON nesting.

Use ``json-metadata`` with the CLI to request the versioned form:

.. code-block:: console

   snt dip parse --input file settings.dip --generate json-metadata settings.json

Each metadata object has a ``present`` list and a ``values`` object. Missing
properties are absent from ``values``; explicitly empty scalar metadata appear
as ``""`` and their names remain in ``present``. Thus absence and an explicit
empty annotation remain distinct. Value records use ``presence: "none"`` and
JSON ``null`` for explicit ``none``; fields absent from the evaluated
environment have no value record. ``node_metadata`` contains records only for
nodes with at least one explicitly present metadata property. The schema is
identified by ``snt-dip-values/2``; its ``values`` and ``sources`` members
retain the version 1 record layouts, with ``node_metadata`` added for complete
hierarchy coverage. For the command-oriented C++ interface, see
:ref:`the DIP API guide <module-api-dip>`.

The legacy values-only JSON generator omits explicit ``none`` values to retain
its existing output contract. The ``JSON_METADATA`` format includes them as
records with ``presence: "none"`` and a JSON ``null`` value. Identity-preserving
links are represented as records with ``type: "link"``, ``presence: "link"``,
a null value, and an explicit ``link`` object containing the source name and
target path; their target data is not duplicated.
