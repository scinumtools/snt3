Introduction
============

Scientific software is built around numerical data, but numerical values
rarely exist in isolation. A value may have a physical unit, an uncertainty,
a specific type, constraints on its valid range, or a relationship to other
values. In larger applications, these properties are often handled by
different mechanisms: units may be represented by a dedicated library,
parameters by configuration files, validation by application code, and
derived values by custom expressions. This fragmentation makes scientific
software harder to develop, maintain, and integrate.

SciNumTools v3 (SNT) is designed to provide a common foundation for these
different aspects of scientific data. It provides a structured and
consistent representation of values, expressions, physical quantities, and
input parameters, allowing their meaning and relationships to be preserved
throughout the software stack. Instead of treating scientific parameters as
simple values accompanied by external metadata, SNT makes their properties
part of the data model itself.

Two domain-specific languages form the conceptual core of SciNumTools.
:doc:`PUEL <puel/index>` (Physical Units Expression Language) provides a compact language for
representing physical quantities and unit expressions, including dimensions,
unit systems, prefixes, uncertainties, arrays, and mathematical operations.
:doc:`DIPL <dipl/index>` (Dimensional Input Parameter Language) builds on these concepts to
describe complete scientific input parameters, including their types, units,
defaults, constraints, options, provenance, expressions, and dependencies.

The underlying framework is implemented primarily in modern C++ and is
organized into modular components. :ref:`VAL <val-values>` provides the
fundamental value system, :ref:`EXS <exs-expression-solver>` provides the
expression-solving infrastructure, :ref:`PUQ <puq-physical-units-and-quantities>`
provides physical quantities and units, and :ref:`DIP <dip-dimensional-input-parameters>`
combines these capabilities into a structured parameter system. The
:ref:`API <api-application-interface>` provides standardized interfaces for
using these capabilities through external applications and services. The
same module structure is exposed through the C++ namespaces and corresponding
Python modules, providing a consistent conceptual API across languages.

A key objective of SciNumTools is to make the same scientific definitions
usable across different environments. PUEL and DIPL definitions can be
consumed by C++ and Python applications and exposed through interfaces such
as command-line tools and REST services. This makes it possible to define
scientific data once and reuse it throughout a workflow instead of
reimplementing the same units, parameter definitions, expressions, and
constraints for every application.

SciNumTools v3 is a substantial architectural development of the original
`SciNumTools v2 <https://github.com/vrtulka23/scinumtools>`_ project. While v2
established the core scientific concepts in a Python-based framework, v3
moves the fundamental functionality into a compiled, language-independent
C++ core while retaining high-level access through Python and other
interfaces.

The result is not simply a new version of the same library, but a
restructuring of the original concepts into a modular scientific data
infrastructure. The v3 architecture provides a foundation that can be
extended from basic numerical values and physical quantities towards
structured input parameters and increasingly complex scientific domains.

This documentation introduces the concepts and languages behind
SciNumTools, followed by guides for the individual modules, practical
examples, installation instructions, and detailed API references. If you
are new to the project, start with :doc:`installation`, followed by
:doc:`puel/index` and :doc:`dipl/index`.
