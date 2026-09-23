# Validity and Errors

This section defines when a PUEL expression is valid independently of the
particular API used to parse it.

## Lexical and structural validity

A standalone PUEL expression MUST conform to the grammar in
[`grammar.ebnf`](../grammar.ebnf). In particular:

* it MUST contain at least one value, unit factor, named entity, or grouped
  expression;
* whitespace is forbidden, including after an array comma;
* an array literal is one-dimensional and MUST contain at least one scalar
  value;
* a fractional exponent denominator MUST be non-zero;
* a system identifier, when present, MUST be the first token and MUST be
  followed by `_`; and
* `+` and `-` are signs only in standalone PUEL. Addition and subtraction
  belong to the spaced PUQ calculator syntax.

An implementation MUST reject malformed delimiters, unknown identifiers,
invalid numerical literals, malformed uncertainties, and unbalanced grouping
parentheses. It MUST NOT silently reinterpret an unknown identifier as a
prefix-plus-unit combination.

## Vocabulary and system validity

Each unit, prefix combination, constant, quantity scale, and system factor
MUST resolve unambiguously in the selected unit system. A prefix is valid only
when the selected unit explicitly permits that prefix. Custom symbols follow
the same uniqueness rule and do not accept prefixes in the PUQ reference
implementation. PUQ may accept an optional space after a literal array comma
as a non-portable input convenience.

Without an explicit system identifier, resolution uses the active unit system.
If an API separately supplies a system and the expression contains a different
system identifier, the expression is invalid because its interpretation would
be ambiguous.

## Dimensional and shape validity

Parsing establishes dimensions but does not by itself request a conversion.
Operations and conversions impose the following additional requirements:

* addition and subtraction require compatible dimensions;
* conversion requires compatible dimensions, unless a documented
  cross-system quantity context is supplied;
* a quantity used as an exponent in a PUQ mathematical operation MUST be
  dimensionless; and
* elementwise array operations require compatible array shapes.

The literal PUEL grammar deliberately defines only one-dimensional arrays.
PUQ can also process multidimensional values supplied through the host VAL/C++
API; that capability is not a multidimensional PUEL literal syntax.

## Diagnostics

Implementations SHOULD report the failing expression or unit, the reason it is
invalid, and a corrective action. The PUQ reference implementation distinguishes
parse failures, unknown or disallowed prefixes, unit-system conflicts,
dimension mismatch, and unavailable special-unit conversions.
