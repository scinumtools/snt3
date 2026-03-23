# PUEL Specification

Version: 1.0  
Status: Stable (Implementation-defined semantics)  
Date: 2026-03-23  

Project: SciNumTools v3  
Reference Implementation: scinumtools.units (PUQ)

---

## 1. Overview

PUEL (Physical Units Expression Language) is a domain-specific language for representing and manipulating physical units and dimensional expressions.

PUEL defines the syntax and structure of unit expressions used in scientific computations. Evaluation, normalization, and conversion of unit expressions are performed by the EXS solver and PUQ module.

PUEL can be used:
- as part of DIPL for unit-aware parameters
- as a standalone unit expression language
- as a backend for programmatic APIs (e.g. Python bindings)

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

All unit expressions are based on a fixed set of base dimensions:

- Length (`m`)
- Mass (`g`)
- Time (`s`)
- Temperature (`K`)
- Charge (`C`)
- Luminous intensity (`cd`)
- Amount of substance (`mol`)
- Angle (`rad`)

Derived units are represented as combinations of these base dimensions with an associated magnitude. :contentReference[oaicite:0]{index=0}

---

## 4. Syntax

### 4.1 General Form

A unit expression consists of unit identifiers combined using algebraic operators.

Examples:

```PUEL
m
km
m/s
kg*m2/s2
m3/(kg*s2)
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

Prefixes scale the magnitude of the unit.

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

## 7. Conversions

PUEL supports conversion between units of identical dimensions:

``km`` → ``m``  
``eV`` → ``J``  
``dB`` → ``W``  

Special handling exists for:

- logarithmic units
- temperature units

These require custom conversion rules.
