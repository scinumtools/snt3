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

Language features
-----------------

This overview introduces PUEL's principal language features. The complete
normative specification is provided below.

Trailing ``#`` annotations in the examples below explain each expression; they
are documentation comments and are not part of PUEL syntax.

Quantities with physical units
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A PUEL expression combines a numerical value, unit factors, or both. A value
without units is dimensionless; a unit without a value denotes one unit.
There is no whitespace within an expression.

.. code-block:: text

   m                   # one metre
   9.81*m/s2           # acceleration
   293.15*K            # absolute temperature
   3.452(3)*kg*m/s2    # value with absolute uncertainty

The parenthesized digits in ``3.452(3)`` represent an uncertainty in the last
significant digits, so the uncertainty stays attached to the quantity through
PUQ calculations.

Unit algebra and exponents
^^^^^^^^^^^^^^^^^^^^^^^^^^

Units compose through multiplication, division, grouping parentheses, and
integer, negative, or fractional exponents. Exponents follow the unit or
closing parenthesis directly; PUEL does not use ``^`` or ``**``.
Fractional exponents use ``numerator:denominator`` notation: ``m1:2`` is the
square root of ``m`` and ``s-1:2`` is the inverse square root of ``s``.

.. code-block:: text

   kg*m2/(sr*s2)       # grouped denominator
   m-1*s-2             # negative exponents
   m1:2                # square-root length dimension
   (m/s)2              # exponent applied to a group

These forms retain their dimensional meaning, so equivalent expressions can
be compared and converted even when written differently.

Arrays and elementwise quantities
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Square brackets express numerical arrays. A unit outside the array applies to
every element; compatible scalar and same-shape array operations are
elementwise.

.. code-block:: text

   [2,3.4,5e6]*km/s        # array with common units
   [2.00(20),3.00(30)]*m   # per-element uncertainties
   [20,40.5]*2             # elementwise scalar multiplication

These compact numerical arrays belong to standalone PUEL expressions. DIPL
uses its own typed array syntax and attaches one scalar PUEL unit expression
to the complete array, for example ``velocity float[3] = [1,0,0] m/s``. A
DIPL unit expression must not contain whitespace or a PUEL numerical array.

Dimensions and normalization
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

PUEL resolves each expression to a scale and base dimensions. SciNumTools
uses ``m``, ``g``, ``s``, ``K``, ``A``, ``cd``, ``mol``, and ``rad`` as its
base dimensions. Thus ``1*kg`` and ``1000*g`` have the same dimensional
meaning, while ``m`` and ``s`` do not and cannot be converted.

This normalization allows PUQ and DIPL to validate dimensional compatibility,
convert values into a requested display unit, and evaluate expressions without
losing their physical interpretation.

Temperature and logarithmic units
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Temperature scales and logarithmic levels require specialized conversions.
They are recognized PUEL units, but cannot be treated as ordinary
multiplicative scaling factors. Absolute temperature conversions can include
an offset, while logarithmic levels can include a reference quantity.

.. code-block:: text

   23*Cel       # converts to 296.15*K
   0*dBm        # referenced power level: 1*mW
   0*dBW        # referenced power level: 1*W

PUQ supports Celsius (``Cel``), Fahrenheit (``degF``), kelvin (``K``), and
Rankine (``degR``), as well as logarithmic ratios and levels including
decibels (``dB``), nepers (``Np``), ``dBm``, and ``dBW``. A bare ``dB`` is a
ratio and does not identify an absolute power; levels such as ``dBm`` and
``dBW`` carry their reference power.

Prefixes and unit systems
^^^^^^^^^^^^^^^^^^^^^^^^^

Supported prefixes express conventional decimal scaling while preserving a
unit's dimensions. PUEL also recognizes unit-system qualifiers at the start
of an expression, separated by an underscore.

.. code-block:: text

   cm              # centi- prefix on metre
   MHz             # mega- prefix on hertz
   US_lb*ft        # US customary mass-length expression
   SI_9.81*m/s2    # explicitly SI acceleration

The available unit systems, units, constants, and allowed prefixes are defined
by the reference tables in the specification. A prefix is accepted only where
the corresponding unit allows it.

Named constants and quantities
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

PUEL can refer to fixed physical constants, active-system constants, physical
quantities, and unit-system scaling factors. Their delimiters make their role
unambiguous inside a larger unit expression.

.. code-block:: text

   {#m_p}          # fixed reference constant
   {N_A}           # active-system constant
   <E>             # physical quantity scale
   |E|             # unit-system conversion factor

Custom units
^^^^^^^^^^^^

The embedding application may register a custom unit from an existing PUEL
expression. Once registered, the symbol behaves like a built-in unit in later
expressions—for example, an application may define ``step`` as ``0.75*m`` and
then evaluate ``4*step``. PUEL itself has no assignment syntax for custom-unit
registration; DIPL provides its ``$unit`` declaration for configuration files.

Specification
-------------

* Specification: `<https://github.com/vrtulka23/scinumtools3/blob/main/docs/puel/specification.md>`_
* Grammar: `<https://github.com/vrtulka23/scinumtools3/blob/main/docs/puel/grammar.ebnf>`_

.. raw:: html

   <iframe
       src="../_static/puel-specification.pdf"
       width="100%"
       height="800px"
       style="border: none;">
   </iframe>
