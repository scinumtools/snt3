# PUEL Specification

Version: 1.0  
Status: Stable (Implementation-defined semantics)  
Date: 2026-03-23  

Project: SciNumTools v3  
Reference Implementation: scinumtools.units (PUQ)

---

## 1. Overview

PUEL (Physical Units Expression Language) is a domain-specific language for representing and manipulating physical units and dimensional expressions.

It defines the syntax and structure of unit expressions used in scientific computations within the [SciNumTools3](https://github.com/vrtulka23/scinumtools3) framework. Evaluation, normalization, dimensional analysis, and unit conversion are performed by the underlying EXS and PUQ modules.

The language is designed for the following use cases:
* As a component of DIPL for defining and validating unit-aware parameters.
* As a standalone language for the concise and machine-readable representation of physical unit expressions.
* As a backend for programmatic interfaces, such as Python bindings and other language integrations.

---

## 2. Design Principles

PUEL is designed to be:

- **Concise** — compact textual representation of units  
- **Composable** — units can be combined using algebraic operations  
- **Deterministic** — expressions resolve to a canonical dimensional form  
- **Extensible** — supports custom units and prefixes  
- **Solver-backed** — evaluation delegated to EXS  

---

## 3. Unit Model

### 3.1. Base units

All unit expressions are ultimately represented in terms of a fixed set of base dimensions:

* Length in meters (`m`)
* Mass in grams (`g`)
* Time in seconds (`s`)
* Temperature in kelvins (`K`)
* Electric charge in coulombs (`C`)
* Luminous intensity in candelas (`cd`)
* Amount of substance in moles (`mol`)
* Angle in radians (`rad`)

Derived units are represented as combinations of these base dimensions together with an associated scaling factor.

The choice of base dimensions is driven by implementation considerations and does not exactly match the SI base-unit system.

 * Mass is represented using grams (`g`) rather than kilograms (`kg`) to avoid embedding a prefix in a base unit.
 * Electric charge is represented using coulombs (`C`) rather than amperes (`A`), making current a derived quantity (`C/s`).
 * Angle is included as a base dimension to enable dimensional validation of mathematical functions that require angular arguments.

### 3.1. Scaling factors

Scaling prefixes provide a compact way to represent decimal multiples and submultiples of units. A scaling prefix modifies a unit identifier by applying a fixed numerical factor while preserving the unit's dimensionality.

For example, the expressions `m`, `cm`, and `km` all represent units of length and differ only in their scaling factors relative to the base unit.

PUEL defines a standard set of scaling prefixes. Implementations MUST recognize and correctly apply these prefixes when parsing unit expressions. However, prefixes SHOULD only be permitted for units for which their use is conventional or widely accepted.

The complete list of supported scaling prefixes is provided in [Appendix A.1: Scaling Prefixes](#a1-scaling-prefixes).

### 3.2. Derived Entities

Derived entities in PUEL include all units (e.g., Joule `J`, degree Celsius `Cel`, decibel `dB`), physical constants (e.g., proton mass `{m_p}`, Avogadro constant `{N_A}`, solar luminosity `{L_sol}`), physical quantities (e.g., energy `<E>`, electric flux `<Phi_E>`, radiation dose `<D_r>`), and unit-system scaling factors (e.g., `|E|`, `|Phi_E|`, `|D_r|`). The dimensionality of these entities is defined in terms of the base dimensions described in the previous section.

PUEL distinguishes between these categories using the following notation:

* Units are represented by their symbols: `J`, `Cel`, `dB`
* Physical constants are enclosed in curly braces: `{m_p}`, `{N_A}`, `{L_sol}`
* Physical quantities are enclosed in angle brackets: `<E>`, `<Phi_E>`, `<D_r>`
* Unit-system scaling factors are enclosed in pipe symbols: `|E|`, `|Phi_E|`, `|D_r|`

Unit-system scaling factors represent the numerical coefficients required to convert a physical quantity between the SI system and another system of units. They are primarily intended to support conversions between systems with different dimensional conventions, such as SI and CGS.

Each implementation MUST define the numerical values and dimensional representations of all supported derived entities. These definitions SHOULD be precomputed and stored to avoid repeated evaluation and dimensional derivation during the processing of unit expressions.

Within a given implementation and unit system, all supported identifiers and scaling-prefix combinations MUST be unique. Consequently, every valid token in a PUEL expression MUST have exactly one interpretation.

---

## 4. Unit expressions

### 4.1 General Form

A unit expression consists of an optional unit system identifier, an numerical value, and a unit expression.

General form:

```text
[<system>_][<value>*][<unit-expression>]
```

where:

* `<system>` is a unit system identifier.
* `<value>` is a numerical value, optionally including an uncertainty.
* `<unit-expression>` is a combination of unit identifiers and algebraic operators.

At least one of `<value>` or `<unit-expression>` MUST be present.

Examples:

```PUEL
m
2.5*m
3.452(3)*kg*m/s2
US_lb*ft
SI_9.81*m/s2
```

Although numerical values are typically written at the beginning of an expression, they are not restricted to that position. Numerical factors may also appear within a unit expression as multiplicative constants. For example, `2*kg*m/s2` and `kg*(2*m)/s2` are equivalent. However, placing numerical values at the beginning of the expression is RECOMMENDED, as it improves readability and simplifies parsing.

The following rules apply:

| Rule                                                                                                                                                                                                                         | Examples                                                                                                                           |
| ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| Expressions MUST NOT contain whitespace; all tokens are written without spaces.                                                                                                                                              | `kg*m/s2`                                                                                                                          |
| Unit identifiers MAY be combined using multiplication (`*`), division (`/`), and grouping parentheses (`(`, `)`).                                                                                                            | `kg*m`<br>`m/s`<br>`kg*m2/(sr*s2)`                                                                                                 |
| Exponents are written directly after the corresponding unit identifier and do not use exponentiation operators such as `^` or `**`.                                                                                          | `m2`                                                                                                                               |
| Fractional exponents are written using the `numerator:denominator` notation.                                                                                                                                                 | `kg3:2`                                                                                                                            |
| Negative exponents are written using a leading minus sign.                                                                                                                                                                   | `s-1`                                                                                                                              |
| A unit system MAY be specified at the beginning of an expression and is separated from the remainder of the expression by an underscore (`_`).                                                                               | `US_lb*ft`                                                                                                                         |
| Numerical values MAY be included in expressions.                                                                                                                                                                             | `2.34e3*mol`<br>`-9.81*m/s2`                                                                                                       |
| The symbols `+` and `-` denote the sign of a numerical value or exponent; they do not represent addition or subtraction operators.                                                                                           | `+5*m`<br>`-5*m`<br>`s-2`                                                                                                          |
| Uncertainties are written in parentheses immediately following the last decimal digit of a numerical value. The number of digits in the uncertainty corresponds to the same number of least significant digits in the value. | `3.45234(2)e3` → `3452.34 ± 0.02`<br>`3.45234(12)` → `3.45234 ± 0.00012`<br>`12.3(4)e-2` → `0.123 ± 0.004`<br>`120(5)` → `120 ± 5` |

## 8. Conversions

PUEL supports conversion between units of identical dimensions:

``km`` → ``m``  
``eV`` → ``J``  
``dB`` → ``W``  

Special handling exists for:

- logarithmic units
- temperature units

These require custom conversion rules.

## Appendinx A: Tables

### A.1 Scaling prefixes

| Prefix | Factor  | Name  |
| ------ | ------- | ----- |
| `Y`    | `1e24`  | yotta |
| `Z`    | `1e21`  | zetta |
| `E`    | `1e18`  | exa   |
| `P`    | `1e15`  | peta  |
| `T`    | `1e12`  | tera  |
| `G`    | `1e9`   | giga  |
| `M`    | `1e6`   | mega  |
| `k`    | `1e3`   | kilo  |
| `h`    | `1e2`   | hecto |
| `da`   | `1e1`   | deka  |
| `d`    | `1e-1`  | deci  |
| `c`    | `1e-2`  | centi |
| `m`    | `1e-3`  | milli |
| `u`    | `1e-6`  | micro |
| `n`    | `1e-9`  | nano  |
| `p`    | `1e-12` | pico  |
| `f`    | `1e-15` | femto |
| `a`    | `1e-18` | atto  |
| `z`    | `1e-21` | zepto |
| `y`    | `1e-24` | yocto |

> [!NOTE]
> The prefix `u` is used to represent micro (`μ`) in ASCII-only unit expressions.


