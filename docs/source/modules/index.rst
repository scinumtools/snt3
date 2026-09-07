Modules
=======

SciNumTools v3 is organized as a modular C++ framework in which the individual
components build upon each other to provide a common infrastructure for
scientific data. The architecture separates fundamental value and expression
handling from higher-level concepts such as physical quantities, dimensional
input parameters, and eventually materials.

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

VAL is used as a building block by higher-level modules, in particular DIP.
Rather than implementing its own independent data representation, the
dimensional input parameter system can use VAL to represent parameter values
and perform operations on them. This establishes a common value model
throughout SciNumTools and allows numerical data to be handled consistently
across the different components and language interfaces.

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

The ``PUQ`` unit solver and calculator are built on EXS to provide
unit-aware expressions and calculations. ``DIP`` similarly uses EXS for
its numerical and logical solvers, including expressions used for
parameter evaluation, conditions, and dependencies. EXS therefore forms
a common expression-processing layer shared by several higher-level
SciNumTools modules.

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

Module Architecture
-------------------

The modules form a layered system rather than a collection of unrelated
libraries. VAL provides the fundamental value layer, while EXS provides
expression evaluation and PUQ provides physical quantities and units. DIP
combines these capabilities into a higher-level scientific parameter system,
and future modules such as MAT can build on the same infrastructure for
specialized scientific domains.

The module structure is preserved across the different language interfaces.
In C++, each module is represented by its corresponding namespace, while in
Python the same structure is exposed as separate modules. This provides a
consistent conceptual organization across languages and makes it possible
to move between C++ and Python without having to learn a fundamentally
different API structure.

This architecture ensures that numerical values, expressions, units,
parameters, and domain-specific data share the same underlying
representation and computational infrastructure throughout SciNumTools.
The result is a coherent framework in which functionality can be combined
across modules while remaining clearly separated by responsibility.
