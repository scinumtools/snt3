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

* Specification: `<https://github.com/vrtulka23/scinumtools3/blob/main/docs/dipl/specification.md>`_
* Grammar: `<https://github.com/vrtulka23/scinumtools3/blob/main/docs/dipl/grammar.ebnf>`_

Specification
-------------

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

* Syntax highlighters: `<https://github.com/vrtulka23/scinumtools3/blob/main/docs/dipl/highlight>`_
