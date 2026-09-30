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

* :doc:`Quantities and arithmetic <quantities>` — construct quantities,
  retain units and uncertainties, and use dimensional arithmetic.
* :doc:`PUEL calculation <calculation>` — evaluate unit-aware expressions
  with the PUQ calculator.
* :doc:`Conversion and unit systems <conversion>` — convert compatible
  quantities and work with unit-system context.

.. toctree::
   :maxdepth: 1
   :hidden:

   quantities
   calculation
   conversion

