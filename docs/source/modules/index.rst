C++ Modules
===========

SciNumTools v3 is a layered C++ framework. ``CORE`` supplies shared
infrastructure, ``VAL`` represents scalar and array values, and ``EXS``
evaluates expressions. ``PUQ`` adds physical quantities and units. ``DIP``
combines these capabilities to define and evaluate scientific input
parameters, while ``API`` exposes them to applications and services. ``MAT``
is planned as a future module for materials and chemical composition.

The module structure appears in the C++ namespaces and, where exposed, the
Python modules. The sections below describe each C++ module and link to its
usage guides.

.. image:: ../_static/module-dependencies.svg
   :alt: Dependency flow from CORE through VAL and EXS to PUQ, then to DIP, MAT, and API.
   :width: 100%

CORE — Core Infrastructure
--------------------------

The ``CORE`` module provides common infrastructure shared throughout
SciNumTools. It contains fundamental data structures, utilities, and
functionality required by the other modules. CORE forms the lowest-level
foundation of the framework.

.. _val-values:

VAL — Values
------------

The ``VAL`` (Values) module provides the fundamental value and data-handling
functionality used by the SciNumTools system. It can be viewed conceptually as
a scientific-data equivalent of NumPy within the framework: it provides the
basic mechanisms for representing, storing, manipulating, and operating on
scalar and array-like values.

VAL supplies a common representation for numerical values across the
higher-level modules, including DIP parameter values.

.. _exs-expression-solver:

EXS — Expression Solver
-----------------------

The ``EXS`` (Expression Solver) module provides the general parsing and
evaluation infrastructure for expressions in SciNumTools. It is designed
as a flexible foundation on which specialized expression solvers can be
built by defining the available operations and their evaluation order.

This makes it possible to create domain-specific expression languages
without implementing a separate parser and evaluation engine for each
application. Operations may represent numerical, logical, mathematical,
or domain-specific functionality, while EXS takes care of their
interpretation and evaluation.

PUQ and DIP build their specialized solvers on EXS. Its configurable parser
and evaluation steps let those modules share expression infrastructure.

* :doc:`Using the EXS solver <exs/basic-usage>` — evaluate an expression with
  the built-in atom and operators.
* :doc:`Operators and evaluation order <exs/operators-and-order>` — choose
  operator symbols and specify the order in which operations run.
* :doc:`Custom atoms and operations <exs/custom-operations>` — add a new
  operator, its evaluation step, and application-specific atom behavior.
* :doc:`Solver settings <exs/settings>` — pass application data to atom
  parsing and operator evaluation.
* :doc:`EXS examples <../examples/exs>` — runnable default, modified, and
  custom solvers.

.. toctree::
   :maxdepth: 1
   :hidden:

   exs/basic-usage
   exs/operators-and-order
   exs/custom-operations
   exs/settings

.. _puq-physical-units-and-quantities:

PUQ — Physical Units and Quantities
-----------------------------------

The ``PUQ`` (Physical Units and Quantities) module extends the numerical
foundation with physical dimensions, units, and uncertainties. It provides
representations of physical quantities and supports unit conversion,
dimensional analysis, arithmetic, uncertainty propagation, and unit-aware
expressions.

PUQ supports multiple unit systems, including SI, US customary, and
electrostatic (ESU) systems, together with unit prefixes and conversions
between compatible units. Physical quantities can therefore retain their
units and associated uncertainty throughout calculations rather than
treating them as external metadata.

PUQ is closely connected to PUEL (Physical Units Expression Language),
which provides a formal language for describing physical quantities and
unit expressions. The PUQ unit solver and calculator are built on the
``EXS`` expression-solving infrastructure, allowing unit expressions and
calculations to be parsed and evaluated using the same general mechanism
as other SciNumTools expression languages.

* :doc:`Quantities and arithmetic <puq/quantities>` — construct quantities,
  retain units and uncertainties, and use dimensional arithmetic.
* :doc:`PUEL calculation <puq/calculation>` — evaluate unit-aware expressions
  with the PUQ calculator.
* :doc:`Conversion and unit systems <puq/conversion>` — convert compatible
  quantities and work with unit-system context.

.. toctree::
   :maxdepth: 1
   :hidden:

   puq/quantities
   puq/calculation
   puq/conversion

.. _dip-dimensional-input-parameters:

DIP — Dimensional Input Parameters
----------------------------------

The ``DIP`` (Dimensional Input Parameters) module brings the individual
capabilities of SciNumTools together into a powerful system for defining
and working with scientific input data. Through DIPL (Dimensional Input
Parameter Language), parameters become more than simple numbers: they can
carry types, units, defaults, constraints, expressions, and dependencies
within a single consistent definition.

DIP turns a collection of loosely managed input values into a structured,
self-describing parameter model. Relationships between parameters can be
expressed directly, allowing derived values and logical conditions to be
defined alongside the parameters themselves rather than hidden in
application code.

Built on VAL, EXS, and PUQ, DIP provides a common foundation for
scientific applications where input data needs to be expressive,
consistent, and reproducible. The result is a parameter system that can
capture not only *what* a value is, but also *what it means*, *how it is
calculated*, and *what constraints it must satisfy*.

* :doc:`Basic DIP usage <dip/basic-usage>` — C++ parsing, environments,
  and input sources.
* :doc:`DIPfile projects <dip/projects>` — reusable manifests for complete
  parameter environments.
* :doc:`Environment persistence <dip/persistence>` — saving and loading
  evaluated environments in DIPH5 format from C++.
* :doc:`Traceability and source identities <dip/traceability>` — following
  DIPL inputs and registered constructs through diagnostics and DIPH5.
* :doc:`Static parameter generation <dip/generation>` — exporting evaluated
  environments as native source code or data files.
* :doc:`C++ report generation <dip/report>` — Brief++ reports of
  evaluated environments.

.. toctree::
   :maxdepth: 1
   :hidden:

   dip/basic-usage
   dip/projects
   dip/persistence
   dip/traceability
   dip/generation
   dip/report

.. _mat-materials:

MAT — Materials
---------------

The ``MAT`` (Materials) module is planned as a future extension of
SciNumTools. Its purpose is to provide functionality for representing
chemical elements, compounds, materials, and mixtures and for calculating
their properties from chemical formulas and composition.

A major objective of MAT is to make chemical composition a directly usable
source of scientific data. A chemical formula could be parsed into its
constituent elements and proportions, allowing properties such as molar mass,
elemental composition, and other composition-dependent quantities to be
calculated automatically. The module is intended to build on the numerical
and dimensional infrastructure already provided by the lower layers of
SciNumTools.

.. _api-application-interface:

API — Application Interface
----------------------------

The ``API`` module provides a standardized interface for exposing the
functionality of SciNumTools to external applications and systems. Its goal
is to provide a consistent way to work with PUEL and DIPL definitions
regardless of the environment in which they are used, separating the
scientific data model from the particular interface used to access it.

The API is intended to support different interfaces such as command-line
applications, console-based tools, REST services, and other integrations.
This allows the same PUEL quantities and DIPL parameter definitions to be
used consistently across different applications and services, providing a
common interface for defining, querying, evaluating, and exchanging
scientific data.

The ``snt::api`` C++ module provides command objects for these workflows.
They return formatted text suitable for interfaces such as the CLI. Code that
needs typed results can use ``snt::puq`` or ``snt::dip`` directly.

* :doc:`PUQ commands <api/puq>` — evaluate, convert, and inspect PUEL quantities and list definitions.
* :doc:`DIP commands <api/dip>` — parse DIPL, load and save DIPH5, and generate static parameters and reports.

.. toctree::
   :maxdepth: 1
   :hidden:

   api/puq
   api/dip
