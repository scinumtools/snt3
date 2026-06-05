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

For example, the expressions m, cm, and km all represent units of length, differing only by their scaling factors relative to the base unit.

PUEL defines the following set of scaling prefixes. Implementations MUST recognize and correctly apply these prefixes when parsing unit expressions.

---

## 4. Syntax

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

The following rules apply:

* Expressions MUST NOT contain whitespace; all tokens are written without spaces.

  Example:

  ```PUEL
  kg*m/s2
  ```

* Unit identifiers MAY be combined using multiplication (`*`), division (`/`), and grouping parentheses (`(`, `)`).

  Examples:

  ```PUEL
  kg*m
  m/s
  kg*m2/(sr*s2)
  ```

* Exponents are written directly after the corresponding unit identifier and do not use exponentiation operators such as `^` or `**`.

  Example:

  ```PUEL
  m2
  ```

* Fractional exponents are written using the `numerator:denominator` notation.

  Example:

  ```PUEL
  kg3:2
  ```

* Negative exponents are written using a leading minus sign.

  Example:

  ```PUEL
  s-1
  ```

* A unit system MAY be specified at the beginning of an expression and is separated from the remainder of the expression by an underscore (`_`).

  Example:

  ```PUEL
  US_lb*ft
  ```

* Numerical values MAY be included in expressions.

  Examples:

  ```PUEL
  2.34e3*mol
  -9.81*m/s2
  ```

* The symbols `+` and `-` denote the sign of a numerical value or exponent; they do not represent addition or subtraction operators.

  Examples:

  ```PUEL
  +5*m
  -5*m
  s-2
  ```

* Uncertainties are written in parentheses immediately following the last decimal digit of a numerical value. The number of digits in the uncertainty corresponds to the same number of least significant digits in the value.

  Examples:

  ```PUEL
  3.45234(2)e3
  3.45234(12)
  12.3(4)e-2
  120(5)
  ```

  Interpretation:

  ```text
  3.45234(2)e3 = 3452.34 ± 0.02
  3.45234(12)  = 3.45234 ± 0.00012
  12.3(4)e-2   = 0.123 ± 0.004
  120(5)       = 120 ± 5
  ```


### 4.2 Operators

PUEL supports the following operators:

Operator	Meaning

``*``	Multiplication  
``/``	Division  
exponent	Power (integer or fractional)  

### 4.3 Exponents
Integer exponents

```PUEL
m2
s-2
```

Fractional exponents

```PUEL
m1:2     # equivalent to m^(1/2)
s-3:2
```

Fractional exponents are represented as numerator:denominator.

### 4.4 Numeric Factors

Unit expressions may include scalar factors:

```PUEL
60*s
365.25*day
1.67e-24*g
```

### 4.5 Parentheses

Parentheses define grouping:

```PUEL
m3/(kg*s2)
```

### 4.6 Prefixes

Units may include prefixes:

```PUEL
km
mm
MJ
```

Prefixes scale the numerical value of the unit.

### 4.7 Constants

Physical constants may be referenced symbolically:

```PUEL
[c]      # speed of light
[k]      # Boltzmann constant
```

### 4.8 Whitespace

Unit expressions MUST NOT contain whitespace.

```PUEL
# valid
kg*m/s2

# invalid
kg * m / s2
```

## 5. Semantics

### 5.1 Dimensional Composition

**Multiplication** → adds dimension exponents  
**Division** → subtracts dimension exponents  
**Power** → multiplies exponents  

### 5.2 Canonical Form

Each unit expression resolves to:

a dimension vector  
a magnitude scaling factor  

Example:

``km`` → ``m`` with factor 1000

### 6.3 Compatibility Rules

**Addition/subtraction** require identical dimensions  
**Multiplication/division** always valid  
**Exponent** must be dimensionless  

## 7. Tables

### 7.1 Scaling prefixes

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


## 8. Conversions

PUEL supports conversion between units of identical dimensions:

``km`` → ``m``  
``eV`` → ``J``  
``dB`` → ``W``  

Special handling exists for:

- logarithmic units
- temperature units

These require custom conversion rules.
