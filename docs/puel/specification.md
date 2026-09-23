---
title: "Physical Units Expression Language"
subtitle: "PUEL Specification for SciNumTools v3"
author: "Ondrej Pego Jaura"
version: "0.1.0"
date: "September 2026"
titlepage: true
---

# PUEL Specification

## Overview

PUEL (Physical Units Expression Language) is a domain-specific language for representing and manipulating physical units and dimensional expressions.

It defines the syntax and structure of unit expressions used in scientific computations within the [SciNumTools3](https://github.com/vrtulka23/scinumtools3) framework. Evaluation, normalization, dimensional analysis, and unit conversion are performed by the underlying EXS and PUQ modules.

The language is designed for the following use cases:

* As a component of DIPL for defining and validating unit-aware parameters.
* As a standalone language for the concise and machine-readable representation of physical unit expressions.
* As a backend for programmatic interfaces, such as Python bindings and other language integrations.

This chapter defines the language syntax and conversion rules. The following chapters provide reference tables for [scaling prefixes](tables/prefixes.md), [unit systems](tables/systems.md), and [shared definitions](tables/common.md), followed by the derived units, constants, and quantity scales of each supported system.

Focused normative sections define [validity and errors](sections/validity.md),
[uncertainty propagation](sections/uncertainty.md), and [special-unit
semantics and conformance options](sections/special-units.md).

### Terminology

A **unit expression** describes units or dimensions, such as `kg*m*s-2`. A
**quantity expression** additionally supplies a value, such as `12*kg*m*s-2`.
Both are standalone, whitespace-free PUEL expressions. A **calculator
expression**, such as `12*kg + 3*g`, is a PUQ operation over complete PUEL
quantities and is not part of the standalone PUEL grammar. Host APIs may also
construct quantities from numerical arrays or values without first parsing a
PUEL literal.

---

## Design Principles

PUEL is designed to be:

- **Concise** — compact textual representation of units  
- **Composable** — units can be combined using algebraic operations  
- **Deterministic** — expressions resolve to an unambiguous dimensional meaning
- **Extensible** — supports custom units and prefixes  
- **Solver-backed** — evaluation delegated to EXS  

### Relationship to Existing Unit Standards

PUEL is a machine-readable language for representing physical quantities and
unit expressions, rather than a replacement for established unit coding
systems. Its vocabulary and dimensional model are designed to be understandable
alongside SI, UCUM, UDUNITS, QUDT, and the OM ontology. PUEL additionally
expresses numerical values, uncertainties, physical constants, symbolic
quantities, and algebraic unit factors.

SciNumTools3 does not currently claim complete bidirectional conversion to
those external serializations. A PUEL expression remains semantically
independent of any particular external notation or ontology.

### Portable PUEL profile

For exchange between implementations, producers SHOULD use the portable
profile: ASCII syntax, SI linear units, no custom symbols, no system-specific
constants or quantity scales, and no temperature or logarithmic units. Use
compact array literals without spaces, such as `[2,3,4]*m`, when arrays are
needed. Producers using an extended feature SHOULD record the required unit
system and feature family with the exchanged data.

---

## Unit Model


### Base units

All unit expressions are ultimately represented in terms of a fixed set of base dimensions:

* Length in meters (`m`)
* Mass in grams (`g`)
* Time in seconds (`s`)
* Temperature in kelvins (`K`)
* Electric current in amperes (`A`)
* Luminous intensity in candelas (`cd`)
* Amount of substance in moles (`mol`)
* Angle in radians (`rad`)

Derived units are represented as combinations of these base dimensions together with an associated scaling factor.

The choice of base dimensions is driven by implementation considerations and does not exactly match the SI base-unit system.

 * Mass is represented using grams (`g`) rather than kilograms (`kg`) to avoid embedding a prefix in a base unit.
 * Electric current is represented using amperes (`A`); electric charge is a derived quantity (`C` = `A*s`).
 * Angle is included as a base dimension to enable dimensional validation of mathematical functions that require angular arguments. However, angular units MUST remain convertible to dimensionless units, reflecting their dimensionless nature in the SI system.


### Scaling factors

Scaling prefixes provide a compact way to represent decimal multiples and submultiples of units. A scaling prefix modifies a unit identifier by applying a fixed numerical factor while preserving the unit's dimensionality.

For example, the expressions `m`, `cm`, and `km` all represent units of length and differ only in their scaling factors relative to the base unit.

PUEL defines a standard set of scaling prefixes. Implementations MUST recognize and correctly apply these prefixes when parsing unit expressions. However, prefixes SHOULD only be permitted for units for which their use is conventional or widely accepted.

The complete list of supported scaling prefixes is provided in [Scaling Prefixes](tables/prefixes.md).

### Named Entities

Named entities in PUEL include all units (e.g., Joule `J`, degree Celsius `Cel`, decibel `dB`), physical constants (e.g., fixed proton mass `{#m_p}`, system-specific Avogadro constant `{N_A}`, solar luminosity `{L_sol}`), physical quantities (e.g., energy `<E>`, electric flux `<Phi_E>`, radiation dose `<D_r>`), and unit-system scaling factors (e.g., `|E|`, `|Phi_E|`, `|D_r|`). The dimensionality of these entities is defined in terms of the base dimensions described in the previous section.

PUEL distinguishes between these categories using the following notation:

* Units are represented by their symbols: `J`, `Cel`, `dB`
* Physical constants are enclosed in curly braces. A leading `#` denotes a
  fixed reference constant, such as `{#m_p}`; without it, the constant is
  resolved in the active unit system, such as `{N_A}` or `{L_sol}`.
* Physical quantities are enclosed in angle brackets: `<E>`, `<Phi_E>`, `<D_r>`
* Unit-system scaling factors are enclosed in pipe symbols: `|E|`, `|Phi_E|`, `|D_r|`

Unit-system scaling factors represent the numerical coefficients required to convert a physical quantity between the SI system and another system of units. They are primarily intended to support conversions between systems with different dimensional conventions, such as SI and CGS.

Each implementation MUST define the numerical values and dimensional representations of all supported derived entities. These definitions SHOULD be precomputed and stored to avoid repeated evaluation and dimensional derivation during the processing of unit expressions.

Within a given implementation and unit system, all supported identifiers and scaling-prefix combinations MUST be unique. Consequently, every valid token in a PUEL expression MUST have exactly one interpretation.

### Custom units

Custom units MAY be defined in the surrounding environment using existing unit expressions. Once registered in a unit system, a custom symbol can be used in expressions just like a built-in unit, including in products, quotients, and powers. For example, an environment may define `step` as `0.75*m` and then interpret `4*step` as `3*m`.

Custom definitions are scoped to their unit system. PUQ stores their numerical scale, uncertainty when present, and dimensions. Registration is performed by the host application; PUEL does not define assignment syntax. PUQ custom units do not accept scaling prefixes.

---

## Unit expressions

### General Form

A unit expression consists of an optional unit system identifier followed by a numerical value, unit factors, or both.

General form:

```text
[<system>_][<value>*][<unit-expression>]
```

where:

* `<system>` is a unit system identifier.
* `<value>` is a numerical value, optionally including an uncertainty.
* `<unit-expression>` is a combination of unit identifiers and algebraic operators.

At least one of `<value>` or `<unit-expression>` MUST be present.

The `*` separator between the value and unit factors is present only when both are supplied. A value alone, such as `2.5`, represents a dimensionless quantity. The value MAY also be a numerical array, as described below.

Examples:

```PUEL
m
2.5*m
3.452(3)*kg*m/s2
US_lb*ft
SI_9.81*m/s2
```

The following rules apply in unit expressions:

| Rule                                                                                                                                                                                                                         | Examples                                                                                                                           |
| ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| Expressions MUST NOT contain whitespace. PUQ MAY accept one space after an array comma as an input convenience, but portable and canonical array literals omit it.                                                             | `kg*m/s2`; `[2,3,4]*m`                                                                                                             |
| Unit identifiers MAY be combined using multiplication (`*`), division (`/`), and grouping parentheses (`(`, `)`).                                                                                                            | `kg*m`; `m/s`; `kg*m2/(sr*s2)`                                                                                                     |
| Exponents are written directly after the corresponding unit identifier and do not use exponentiation operators such as `^` or `**`.                                                                                          | `m2`                                                                                                                               |
| Fractional exponents are written using the `numerator:denominator` notation.                                                                                                                                                 | `kg3:2`                                                                                                                            |
| Negative exponents are written using a leading minus sign.                                                                                                                                                                   | `s-1`                                                                                                                              |
| A unit system MAY be specified. When present, it MUST appear at the beginning of the expression and MUST be separated from the remainder by an underscore (`_`).                                                              | `US_lb*ft`                                                                                                                         |
| Numerical values MAY be included in expressions.                                                                                                                                                                             | `2.34e3*mol`; `-9.81*m/s2`                                                                                                         |
| The symbols `+` and `-` denote the sign of a numerical value or exponent; they do not represent addition or subtraction operators.                                                                                           | `+5*m`; `-5*m`; `s-2`                                                                                                              |
| Uncertainties are written in parentheses immediately following the last decimal digit of a numerical value. The number of digits in the uncertainty corresponds to the same number of least significant digits in the value. | `3.45234(2)e3` → `3452.34 ± 0.02`; `3.45234(12)` → `3.45234 ± 0.00012`; `12.3(4)e-2` → `0.123 ± 0.004`; `120(5)` → `120 ± 5`     |

### Expression Parsing

Although numerical values are typically written at the beginning of an expression, they are not restricted to that position. Numerical factors MAY appear anywhere within a unit expression as multiplicative constants. For example, `2*kg*m/s2` and `kg*(2*m)/s2` are equivalent.

Implementations SHOULD accept numerical factors in any valid position within a unit expression. However, producers of unit expressions SHOULD place any numerical factor at the beginning of the expression, as this improves readability and simplifies parsing.

Unit expressions MAY use division operators and grouping parentheses for readability. For example, `kg*m2/(sr*s2)` and `kg*m2*sr-1*s-2` represent the same unit expression.

An exponent MAY follow a closing parenthesis and applies to the entire grouped expression, including numerical factors. Integer, negative, and fractional exponents use the same notation as unit exponents. For example, `(m/s)2` is equivalent to `m2*s-2`, and `kg/(m*s)-1:2` is equivalent to `kg*m1:2*s1:2`. Exponents also apply to named entities, as in `kg*<v>2`.

Expressions containing only numerical factors are supported. For example, `3*(2.0e1/5.0)` evaluates to the dimensionless value `12`.

### Numerical arrays

A numerical array is written as a comma-separated list of values in square brackets. All elements share the units specified outside the brackets. Elements MAY include scientific notation or parenthesized uncertainties. The portable form contains no spaces; PUQ also accepts one optional space after a comma for input compatibility.

```PUEL
[2,3.4,5e6]*km/s
[2.00(20),3.00(30)]*m
```

Multiplication and division by a scalar apply to each element. Multiplication and division between arrays of the same shape are elementwise operations. For example, `[20,40.5]*2` evaluates to `[40,81]`, and `[20,40.5]*[2,3]` evaluates to `[40,121.5]`. Unit factors are combined using the ordinary expression rules.

### Unit-system selection

The system identifier, when present, MUST appear only at the beginning of the expression, immediately followed by an underscore (`_`). It selects the interpretation of all unit factors in the expression. For example, `US_23*ft2` is valid; `US 23*ft2` and `23*US_ft2` are not valid PUEL expressions. PUQ supports SI and, when enabled, CGS variants (`ESU`, `EMU`, `GU`), English systems (`IU`, `US`), and natural systems (`AU`, `PU`, `SRU`, `GRU`, `GEO`).

When no identifier is supplied, PUQ uses the active unit system of the surrounding environment, initially SI. If both the expression and its environment explicitly select a system for that expression, the selections MUST agree. Producers SHOULD include the system identifier when an expression must retain its interpretation outside its original environment.

### Display notation

In addition to ASCII notation, PUQ can produce Unicode and HTML representations for display. When Unicode input preprocessing is enabled, it accepts superscript exponents, the Unicode middle dot (U+22C5) for multiplication, the Unicode minus sign (U+2212), and the multiplication sign (U+00D7) for scientific notation. For example, a displayed form using U+00D7 before `10` and superscript U+2212/U+22C5 symbols corresponds to the portable expression `6.23537e-2*C3`.

Unicode input conveniences are optional; ASCII notation remains the portable form. A system identifier MUST retain its leading position and underscore separator even when Unicode notation is used, as in the displayed form of `US_23*ft2`. HTML output is a display format, not an expression input format.

### Normalization and Serialization

Implementations MUST resolve a valid expression to its numerical scale,
dimensions, and applicable special-unit semantics. Division may be normalized
to negative exponents; for example, `kg*m2/(sr*s2)` and
`kg*m2*sr-1*s-2` have the same dimensional meaning.

PUEL does not yet prescribe one unique textual serialization for every
semantically equivalent expression. Implementations may retain factor order or
choose their own normalized display form. They MUST NOT imply that formatting
alone preserves the original placement of division operators or grouping
parentheses.

For portable serialization, producers SHOULD use ASCII notation, place a
numerical factor first, join factors with `*`, use negative exponents rather
than `/`, and omit whitespace from array literals. Implementations that offer
a stronger canonical serialization MUST document their factor ordering,
numeric precision, uncertainty formatting, prefix rebasing, and treatment of
temperature and logarithmic units.

When representing base dimensions, only dimensions with non-zero exponents
SHOULD be included. Their order MUST follow the base-unit list in
[Base units](#base-units): `m`, `g`, `s`, `K`, `A`, `cd`, `mol`, and `rad`.

## Calculator expressions

The grammar in `grammar.ebnf` defines a single, whitespace-free PUEL unit
expression. ``Quantity`` and ``UnitSolver`` parse that form directly; for
example, write `12*km`, not `12 km`.

PUQ also provides a calculator layer for arithmetic between complete PUEL
quantities. Its binary operators are written with spaces around them, so each
operand remains a valid PUEL expression:

```PUEL
23*cm + 3*m
2*g * ( 6*m2/s2 - 3*cm2 / 1.5*s2 )
```

The calculator supports grouping, unary `+` and `-`, and binary `+`, `-`,
`*`, and `/`, in the usual precedence order. Calculator syntax is a PUQ host
operation rather than an additional form accepted by the standalone PUEL unit
solver. Likewise, the C++ `puq::math` functions such as `sqrt`, `pow`, `log`,
and trigonometric functions operate on parsed quantities but do not add
function-call syntax to PUEL.

## Conversions

PUQ conversion APIs operate on quantities parsed from PUEL and can convert
between units of identical dimensions:

``km`` → ``m``  
``eV`` → ``J``  
``dBW`` → ``W``

Special handling exists for:

- logarithmic units
- temperature units

These require custom conversion rules.

### Logarithmic and temperature conversions

Logarithmic conversions distinguish ratios from levels with a defined reference. PUQ supports power ratios (`PR`), amplitude ratios (`AR`), and logarithmic units such as decibels (`dB`) and nepers (`Np`). Power levels such as `dBm` and `dBW` include their reference power: `0*dBm` corresponds to `1*mW`, while `0*dBW` corresponds to `1*W`. A bare `dB` value does not specify an absolute power.

Absolute temperature conversions can include an offset as well as a scale. For example, `23*Cel` converts to `296.15*K`. PUQ supports Celsius (`Cel`), Fahrenheit (`degF`), kelvin (`K`), and Rankine (`degR`). Such conversions cannot be described by a multiplicative scaling factor alone.

### Conversion between unit systems

Conversions between systems with different dimensional conventions may require a physical-quantity context in addition to source and target units. For example, converting SI current in `A` to electrostatic CGS current in `statA` requires the current context `I`. Energy conversion between `J` and `erg` does not require an explicit context.

The context identifies the corresponding physical quantity in each system and enables the use of its unit-system scaling factors. It is supplied by the surrounding application, not appended as an additional operator in the expression. A system prefix selects a system; it does not by itself specify a conversion context.
