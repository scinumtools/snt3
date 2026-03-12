# DIPL - Language Specification

Version: 1.0

Project: SciNumTools v3

Language: DIPL — Dimensional Input Parameter Language

## 1. Introduction

DIPL (Dimensional Input Parameter Language) is a domain-specific language designed for defining typed, unit-aware parameters for scientific simulations and numerical codes.

The language supports:
- dimensional units, unit systems and automatic unit conversions
- typed parameters
- array shapes
- expression solvers
- parameter validation rules
- referencing of parameters

DIPL is intended to replace fragile configuration formats such as JSON, YAML, or INI files when scientific parameters require dimensional correctness, numerical precision and validation.

Typical applications include:
- computational fluid dynamics
- astrophysical simulations
- robotics simulation
- numerical modeling codes
- sensor and satellite data processors

## 2. Language Overview

A DIPL file contains parameter declarations arranged in hierarchical blocks using indentation.

Example:

```
velocity float32[1,2:] = [[23.45, 23e-34, 45.1]] SI_km/s
    !condition (" {?} < {?cfl_limit} ")
```

burst_energy float64 = 2.34e5 US_btu

Each parameter declaration may specify:

component	description
|name	  |parameter identifier     |
|type	  |storage type             |
|shape	|dimensional constraints  |
|value	|numeric value, array, reference, or expression   |
|unit	  |physical unit            |
|tags	  |validation or metadata   |

## 3. Parameter Declaration

General form:

```
name type[shape] = value unit
```

Example:

```
density float64 = 1000 kg/m^3
```

Components:

element	meaning
name	parameter identifier
type	numeric storage type
shape	optional array shape
value	literal, expression, array, or reference
unit	physical unit

## 4. Types

DIPL supports typed numerical parameters.

Examples:

float32
float64
int32
int64

The type defines the internal storage representation used by the implementation.

## 5. Shape Specification

Array parameters may define shape constraints.

Example:

velocity float32[1,2:]

Meaning:

first dimension = 1

second dimension ≥ 2

General syntax:

[dim1, dim2, ...]

Dimension forms:

form	meaning
N	fixed size
N:	minimum size
:	unconstrained

Example:

matrix float64[3,3]
vector float64[3:]

## 6. Values

Values may be:

Scalar values
pressure float64 = 101325 Pa
Arrays
vector float64[3] = [1, 2, 3]

Nested arrays represent higher dimensions.

## 7. Units

Units follow the value.

value unit

Example:

gravity float64 = 9.81 m/s^2

DIPL supports multiple unit systems including:

SI

CGS

US

Implementations may normalize units internally.

## 8. Expressions

Expressions are enclosed in parentheses.

Example:

velocity float64 = (" {distance} / {time} ") m/s

Expressions may include:

arithmetic operators

parameter references

numeric literals

Implementations should propagate dimensional units through expressions.

## 9. References

Parameters may reference other parameters.

Syntax:

{source?path}

Example:

{simulation?mesh.dx}

Components:

part	meaning
source	external parameter source
path	hierarchical parameter path

A shorthand self-reference is allowed:

{?}

which refers to the current parameter.

## 10. Metadata Tags

Additional constraints may be attached using tags.

Example:

!condition (" {?} < {?cfl_limit} ")

Tags appear below the parameter declaration and apply to the preceding parameter.

Common tag types may include:

tag	purpose
!condition	validation rule
!description	documentation
!range	numeric constraints

## 11. Execution Model

A DIPL processor evaluates files using the following steps:

parse
 → build parameter tree
 → resolve references
 → evaluate expressions
 → apply unit conversions
 → validate shapes
 → evaluate conditions

If any step fails, an error must be reported.

## 12. Error Handling

DIPL implementations should detect and report:

error type	example
syntax error	invalid declaration
type error	incompatible value type
shape error	array size mismatch
unit error	incompatible units
reference error	unresolved reference
validation error	failed condition

## 13. Example

Complete example:

distance float64 = 10 km
time float64 = 5 s

velocity float64 = (" {distance} / {time} ") km/s
    !condition (" {?} < {?cfl_limit} ")

burst_energy float64 = 2.34e5 US_btu
## 14. Versioning

This document describes DIPL language version 1.0.

Future versions may introduce additional syntax or tags while preserving backward compatibility where possible.

## 15. Reference Implementation

The reference implementation of DIPL is provided in the SciNumTools project.

Repository:

https://github.com/...
