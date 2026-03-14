# DIPL - Language Specification

Version: 0.1  
Status: Draft  
Project: SciNumTools

## Introduction

DIPL (Dimensional Input Parameter Language) is a domain-specific language for
defining typed, unit-aware parameters for scientific simulations and numerical
codes.

The language provides:

- dimensional units
- typed parameters
- array shape constraints
- parameter references
- expressions
- validation rules
- conditional operators

DIPL is intended to replace fragile configuration formats such as JSON, YAML,
or INI files when physical units and validation are required.

## Language Overview

A DIPL file consists of parameter declarations organized in hierarchical
blocks using indentation.

Example:

```dipl
velocity float32[1,2:] = [[23.45, 23e-34, 45.1]] SI_km/s
    !condition (" {?} < {?cfl_limit} ")

burst_energy float64 = 2.34e5 US_btu
```

## Language Syntax

- [Nodes](syntax/nodes.md)
- [Data Types](syntax/datatypes.md)
- [Values](syntax/values.md)
- [References](syntax/references.md)
- [Expressions](syntax/expressions.md)
- [Units](syntax/units.md)
- [Functions](syntax/functions.md)
- [Properties](syntax/properties.md)
- [Conditions](syntax/conditions.md)
 
## Execution Model

A DIPL processor evaluates files using the following steps:

1. parse file
2. build parameter tree
3. resolve references
4. evaluate expressions
5. apply unit conversions
6. validate shapes
7. evaluate conditions

Errors are reported if any step fails.

## Error Handling

Possible error types include:

- runtime error

## Examples

## Versioning

This document describes DIPL version 0.1.

Future versions may extend the language while maintaining backward
compatibility where possible.

## Reference Implementation

The reference implementation of DIPL is provided in the SciNumTools v3 project.

[scinumtools3](https://github.com/vrtulka23/scinumtools3)
[documentation](https://vrtulka23.github.io/scinumtools3/)