DIPL — Dimensional Input Parameter Language
============================================

DIPL (Dimensional Input Parameter Language) provides a compact and
structured language for defining scientific input parameters. It allows
parameters to be described together with their type, value, physical unit,
default value, constraints, and relationships to other parameters. This
turns a collection of input values into a self-describing parameter model
that can be interpreted consistently by different applications.

DIPL also allows parameters to be expressed in terms of other parameters.
Numerical and logical expressions can therefore be used to define derived
values, conditions, and dependencies directly within the parameter
definition. Combined with PUEL and the underlying VAL and EXS modules,
this provides a consistent way to represent both the data and the logic
associated with scientific input.

DIPL is intended to serve as a common input language across different
scientific tools and applications. The same parameter definition can be
used from C++, Python, or C and through command-line and CMake interfaces,
avoiding the need to implement separate parameter handling and validation
logic for each application.

Language features
-----------------

This overview introduces DIPL's main language features. The complete normative
language specification is provided below.

Structured, typed values
^^^^^^^^^^^^^^^^^^^^^^^^

DIPL is indentation-based: two spaces establish a child level, while a dot
in a name gives the equivalent fully qualified path. Comments begin with
``#``. A value definition has a name, explicit type, value, and—when
numeric—an optional PUEL unit:

.. code-block:: dipl

   simulation
     title str = "Cylinder flow"
     time.timestep float = 1e-3 s
     enabled bool = true
     iterations uint = 250

Scalar types include ``bool``, ``int``/``uint``, ``float``, ``str``,
``char``, and ``byte``; fixed-width integer and floating-point types are also
available. ``none`` represents an explicitly empty value. Arrays declare a
shape after their type; bounds such as ``[3:]`` and ``[:4]`` constrain a
dimension. Arrays can be multidimensional and numerical units apply to every
element:

.. code-block:: dipl

   labels str[3:4] = ["inlet", "wall", "outlet"]
   velocity float[3] = [1.0, 0.0, 0.0] m/s
   matrix float[2,2] = [[1.0, 0.0], [0.0, 1.0]]
   note str = """A string may span
   multiple lines."""

Typed tabular data
^^^^^^^^^^^^^^^^^^

The ``table`` type turns a compact, typed table block into child arrays. Its
columns may carry units and a ``!delimiter`` property can select a CSV- or
TSV-like separator:

.. code-block:: dipl

   samples table = """
   time float s
   temperature float K
   ---
   0 293.15
   1 294.20
   """

Groups, maps, and lists
^^^^^^^^^^^^^^^^^^^^^^^

Groups organize named children. Keyed collections are maps and indexed
collections are lists; collections may be nested:

.. code-block:: dipl

   materials[water]
     density float = 998.2 kg/m3
   materials[air]
     density float = 1.225 kg/m3

   sensors[]
     name str = "inlet"
   sensors[]
     name str = "outlet"

References, slices, and templates
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

References use braces. A local reference begins with ``?`` and a named-source
reference puts the source before it. Values may be sliced, complete subtrees
can be imported with a trailing dot, and formatted strings interpolate a
reference with double braces:

.. code-block:: dipl

   speed float = ({?distance} / {?duration})
   first_component float = {?velocity}[0]
   message str = f"Speed: {{?speed}}"

Derived values and decisions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Numerical and logical expressions support arithmetic, comparisons, Boolean
operators, parentheses, exponentiation, and built-in numerical functions.
They establish dependencies which are resolved before values are validated:

.. code-block:: dipl

   area float = ({?width} * {?height})
   reynolds float = ({?density} * {?speed} * {?diameter} / {?viscosity})
   stable bool = (({?reynolds} < 2e3) || ({?reynolds} > 4e3))

Validation, metadata, and provenance
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Properties immediately below a value add validation, display guidance,
categorization, and scientific provenance. ``{.}`` is the normalized value of
the enclosing node and is available in a condition:

.. code-block:: dipl

   temperature float = 293.15 K
     !condition ({.} > 0 K)
     !tags ["initial", "measured"]
     ?descr "Initial fluid temperature"
     ?doi "10.0000/example.dataset"
     ?license "CC BY 4.0"

``!options`` restricts permitted values, ``!format`` validates strings using
a regular expression, and ``!constant`` prevents later modifications. A
definition may also be declared without a value and completed by a later
compatible modification; compatible units are converted to the definition's
unit.

External sources and custom units
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Named sources make external DIPL documents or raw text available without
copying them into the local file. Custom PUEL units can be declared alongside
the parameters:

.. code-block:: dipl

   $source constants = "data/constants.dip"
   $unit cell_length = 2.5 m

   proton_mass float = {constants?proton_mass}
   domain_length float = 4 [cell_length]

Reusable schemas
^^^^^^^^^^^^^^^^

Schemas declare reusable, validated structures. They can be applied to groups,
map items, lists, or complete collections:

.. code-block:: dipl

   $schema material
     density float kg/m3;
     conductivity float W/(m*K);

   materials[copper] : material
     density = 8960 kg/m3
     conductivity = 401 W/(m*K)

Conditional definitions
^^^^^^^^^^^^^^^^^^^^^^^

Conditional blocks select definitions from logical expressions. Only the first
matching branch is evaluated:

.. code-block:: dipl

   @if ({?enabled})
     mode str = "active"
   @else
     mode str = "disabled"
   @end

Applications may additionally register host-defined functions that return
values or node trees. Their names and semantics belong to that application;
all built-in DIPL constructs above have the portable language semantics
defined by the specification.

Specification
-------------

* Specification: `<https://github.com/vrtulka23/scinumtools3/blob/main/docs/dipl/specification.md>`_
* Grammar: `<https://github.com/vrtulka23/scinumtools3/blob/main/docs/dipl/grammar.ebnf>`_

.. raw:: html

   <iframe
       src="../_static/dipl-specification.pdf"
       width="100%"
       height="800px"
       style="border: none;">
   </iframe>
   </br></br>
   
Syntax highlighting
-------------------

The repository includes syntax definitions for the main editors and
documentation tools. The `Pygments lexer and style
<https://github.com/vrtulka23/scinumtools3/tree/main/docs/dipl/highlight/pygments>`_
are used for ``dipl`` code blocks in this Sphinx documentation. An
`Emacs mode <https://github.com/vrtulka23/scinumtools3/tree/main/docs/dipl/highlight/emacs>`_,
`Skylight/Kate definition <https://github.com/vrtulka23/scinumtools3/tree/main/docs/dipl/highlight/skylight>`_,
and `TextMate package <https://github.com/vrtulka23/scinumtools3/tree/main/docs/dipl/highlight/textmate>`_
are provided for editing DIPL files. The shared
`highlighting test fixture <https://github.com/vrtulka23/scinumtools3/blob/main/docs/dipl/highlight/highlighting-test.dipl>`_
covers the language constructs supported by these highlighters.

See the :doc:`rendered Pygments fixture <highlighting>` to verify the Sphinx
highlighting directly.

* Syntax highlighters: `<https://github.com/vrtulka23/scinumtools3/blob/main/docs/dipl/highlight>`_

.. toctree::
   :hidden:

   highlighting

Application
-----------

`Nuclide Atlas <https://github.com/vrtulka23/nuclide-atlas>`_ is a unit-aware
nuclear-isotope database and decay-chain calculator built with SciNumTools3.
It uses DIPL sources for validated nuclide records and scenarios, including
units and provenance metadata, and its CMake build configuration is driven by
DIPL through the SNT CMake integration.

   
