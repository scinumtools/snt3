```{=latex}
\appendix
```

# Formal Grammar

This appendix gives the grammar for a standalone PUEL expression in
extended Backus–Naur form (EBNF). It is the source grammar used by the PUEL
specification and is included here directly from `grammar.ebnf`.

In the grammar, quoted strings are literal characters or tokens. A comma
separates grammar elements; `|` means “or”; square brackets mark an optional
element; and braces mark zero or more repetitions. Whitespace shown between
grammar elements is for readability and is not permitted inside a portable
PUEL expression.

The grammar describes syntactic structure. It does not by itself establish
that an identifier exists in the selected unit system, that a prefix is valid
for a unit, or that an expression satisfies the validity, uncertainty, and
special-unit rules in the preceding chapters. In particular, array literals
must contain at least one scalar value, and the portable form contains no
whitespace. Those constraints and implementation options are specified in
[Validity and errors](#validity-and-errors),
[Uncertainty propagation](#uncertainty-propagation), and
[Special units and conformance options](#special-units-and-conformance-options).

```{.text include="docs/puel/grammar.ebnf"}
```
