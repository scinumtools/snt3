Scientific Numerical Tools v3
=============================

SciNumTools (SNT) is a scientific software framework for defining,
processing, and exchanging numerical and physical input data. It provides
a common foundation for values, expressions, physical quantities, units,
and dimensional input parameters, with the goal of keeping scientific data
consistent and meaningful throughout an application.

At the core of SNT are two domain-specific languages: PUEL (Physical Units
Expression Language) for describing physical quantities and unit
expressions, and DIPL (Dimensional Input Parameter Language) for defining
structured scientific input parameters. Together, they provide a compact
and machine-readable way to describe not only values, but also their units,
types, constraints, relationships, and dependencies.

`SciNumTools v3 <https://github.com/vrtulka23/scinumtools3>`_ is built
around a modular C++ core with Python bindings and
interfaces for command-line tools, CMake, and REST services. The same
scientific definitions can therefore be used across different languages,
applications, and interfaces without duplicating the underlying logic.
This documentation introduces the concepts, languages, modules, and APIs
that make up the SciNumTools framework.

.. toctree::
   :maxdepth: 2
   :caption: Getting Started
   
   introduction
   installation
   quickstart

.. toctree::
   :maxdepth: 2
   :caption: Core Concepts
   
   puel/index
   dipl/index

.. toctree::
   :maxdepth: 2
   :caption: Guides
   
   modules/index
   examples/index

.. toctree::
   :maxdepth: 2
   :caption: Reference
   
   api/index

Previous version
----------------

Compared with `SciNumTools v2 <https://github.com/vrtulka23/scinumtools>`_, which was a Python-native framework integrating expression evaluation, physical units, material properties, and validated dimensional input parameters, SciNumTools v3 represents a substantial architectural evolution. The core functionality is being rebuilt in modern C++ as a compiled, modular foundation, removing the dependency on Python for the fundamental parameter and unit-processing pipeline. This makes the framework suitable not only for Python workflows but also for performance-critical applications, HPC environments, and direct integration with C++ and other languages.

The `v3` development also moves beyond simply porting the `v2` functionality. The concepts are being reorganized around a more explicit language and data model, with `PUEL` (Physical Units Expression Language) providing the foundation for unit-aware expressions and `DIPL` (Dimensional Input Parameter Language) providing structured, validated scientific parameters. The project now includes a C++ core, command-line tooling, Python interfaces, CMake integration, documentation, examples, and testing infrastructure, establishing a foundation on which the higher-level functionality of `v2` can be rebuilt in a more general and extensible form.

In short, `v2` established the scientific concepts and Python implementation; `v3` is turning those concepts into a language-independent, compiled scientific infrastructure. This makes `v3` less of a conventional successor package and more of a redesign of the underlying architecture, with the aim of allowing the same scientific parameter definitions and semantics to be used consistently across C++, Python, command-line tools, simulations, and HPC applications.

Documentation for the original Python implementation, `SciNumTools2`, is available here:

* `SciNumTools2 Documentation <https://vrtulka23.github.io/scinumtools/>`_

.. toctree::
   :maxdepth: 2
   :caption: Contents:
