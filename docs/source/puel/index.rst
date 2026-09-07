PUEL — Physical Units Expression Language
==========================================

PUEL (Physical Units Expression Language) is a compact language for
representing physical quantities, units, and unit expressions in a
machine-readable form. It provides a common syntax for describing
quantities together with their physical dimensions, allowing numerical
values and their units to be processed as a single entity.

PUEL supports different unit systems, including SI, US customary, and ESU,
as well as unit prefixes and conversions between compatible units. It can
therefore represent the same physical quantity using different systems
while retaining its underlying dimensional meaning. Expressions can also
include uncertainties, allowing uncertainty information to remain attached
to the quantity throughout calculations.

PUEL forms the language foundation of the PUQ module and is evaluated using
the EXS expression-solving infrastructure. It is intended to provide a
consistent representation of physical quantities across SciNumTools,
whether they are used in C++, Python, configuration files, command-line
interfaces, or other APIs.

** `PUEL Specification <https://github.com/vrtulka23/scinumtools3/blob/main/docs/puel/specification.md>`_
