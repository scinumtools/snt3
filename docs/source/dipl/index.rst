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
used from C++ or Python and exposed through interfaces such as the
command-line and REST API, avoiding the need to implement separate
parameter handling and validation logic for each application.

 `DIPL Specification <https://github.com/vrtulka23/scinumtools3/blob/main/docs/dipl/specification.md>`_

.. raw:: html

   <iframe
       src="_static/specification.pdf"
       width="100%"
       height="800px"
       style="border: none;">
   </iframe>
