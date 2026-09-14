# DIPL Pygments highlighter

`syntax_lexer.py` provides the `dipl` lexer for Pygments. The lexer is
registered locally by the Sphinx configuration in `docs/source/conf.py` and
is used with:

```rst
.. code-block:: dipl

   length float = 2.5 m
```

`style_lexer.py` contains the matching DIPL token palette. The lexer supports
declarations, directives, branching, values, units, references, arrays,
strings, and expressions from the current DIPL specification.

To use it in another Sphinx project, add this directory to `sys.path`, import
`SyntaxLexer`, and register it through `sphinx.highlighting.lexers['dipl']`.
Apply the token colors from `StyleLexer` through the project’s Pygments/CSS
configuration.

`../highlighting-test.dipl` is a deliberately broad fixture for checking
lexer output across the DIPL syntax.
