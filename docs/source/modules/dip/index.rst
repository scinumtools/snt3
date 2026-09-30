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

* :doc:`Basic DIP usage <basic-usage>` — C++ parsing, environments,
  and input sources.
* :doc:`DIPfile projects <projects>` — reusable manifests for complete
  parameter environments.
* :doc:`Environment persistence <persistence>` — saving and loading
  evaluated environments in DIPH5 format from C++.
* :doc:`DIPH5 comparison <comparison>` — summarized differences between
  evaluated snapshots.
* :doc:`Traceability and source identities <traceability>` — following
  DIPL inputs and registered constructs through diagnostics and DIPH5.
* :doc:`Inspecting and reloading environments <inspection>` — reading
  evaluated values and provenance through a small C++ API.
* :doc:`Static parameter generation <generation>` — exporting evaluated
  environments as native source code or data files.
* :doc:`Application adapters <adapters>` — generating one or more
  application-specific files from an evaluated environment.
* :doc:`C++ report generation <report>` — Brief++ reports of
  evaluated environments.

.. toctree::
   :maxdepth: 1
   :hidden:

   basic-usage
   projects
   persistence
   comparison
   traceability
   inspection
   generation
   adapters
   report
