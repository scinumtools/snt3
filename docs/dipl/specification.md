# DIPL - Language Specification

**Version:** 0.1

**Status:** Draft (17.03.2026)

**Project:** SciNumTools v3

## 1. Introduction

DIPL (Dimensional Input Parameter Language) is a domain-specific language for
defining typed, unit-aware parameters for scientific simulations and numerical
codes.

The language provides:

- dimensional units
- typed parameters
- arrays with shape constraints
- hierarchical parameter structure
- parameter references (node imports and value injections)
- numerical and logical expression solvers
- text template solver
- parameter properties and validation rules
- conditional operators

DIPL is intended to replace fragile configuration formats such as JSON, YAML,
or INI files when physical units, precision and validation are required.

## 2. Language Overview

A DIPL file consists of parameter declarations organized in hierarchical
blocks using indentation.

Example:

```dipl
velocity float32[1,2:] = [[23.45, 23e-34, 45.1]] SI_km/s
    !condition (" {?} < {?cfl_limit} ")

burst_energy float64 = 2.34e5 US_btu
```

## 3. Language Syntax

- [3.1. Nodes](syntax/nodes.md)
- [3.2. Data Types](syntax/datatypes.md)
- [3.3. Values](syntax/values.md)
- [3.4. References](syntax/references.md)
- [3.5. Expressions](syntax/expressions.md)
- [3.6. Units](syntax/units.md)
- [3.7. Functions](syntax/functions.md)
- [3.8. Properties](syntax/properties.md)
- [3.9. Conditions](syntax/conditions.md)
 
## 4. File Format

The source file MUST have an extension of either `.dip` or `.dipl`.
Nodes with the smallest [indentations](syntax/nodes.md#3.1.1.-definition) in the file should be considered as the root nodes with an indentation level of 0.

## 5. Execution Model

A DIPL processor evaluates files using the following steps:

1) parse code lines into nodes
   1. combine multiline string [blocks](syntax/values.md#3.3.4.-blocks) into single strings (e.g. text wrapped in ``"""``)
   2. replace special symbols with replacing marks (e.g. ``\n``, ``\'``, ``\"``)
   3. determine a node type for each line (e.g. [group](syntax/nodes.md#3.1.4.-hierarchy), [value](syntax/values.md#3.3.-values), [case](syntax/conditions.md#3.9.-conditions), ...)
   4. convert replacing marks back to corresponding special symbols
   5. populate node list from processed lines
2) prepare node for processing
   1. assign [property](syntax/properties.md#3.8.-properties) nodes to preceding value nodes (e.g. ``!options [2, 3] erg``)
   2. check node [indentation](syntax/nodes.md#3.1.1.-definition) (e.g. number of whitespace characters)
3) parse nodes
   1. perform specific node parsing outside or inside a valid condition block (e.g. expand [table](syntax/values.md#3.3.5.-tables) nodes and solve [references](syntax/references.md#3.4.-references))
   2. solve the node [hierarchy](syntax/nodes.md#3.1.4.-hierarchy) and construct node full names (e.g. ``foo.bar.baz``)
   3. solve current [branching](syntax/conditions.md#3.9.-conditions) and parse case expression
   4. clean value node names from ``@if`` nodes
   5. parse node [value](syntax/values.md#3.3.-values) (e.g. parse scalars ``34e+3`` and arrays ``[[1,3],[4,5]]``)
   6. parse node [units](syntax/units.md#3.6.-units) (e.g. solve unit expressions ``kg/s``)
   7. apply [modifications](syntax/nodes.md#3.1.2.-modification) to already existing nodes
4) validate nodes
   1. check if all nodes are [defined](syntax/nodes.md#3.1.3.-declaration)
   2. check if nodes value is in available [options](syntax/properties.md#3.8.1.-options)
   3. check if nodes fulfill specific [conditions](syntax/properties.md#3.8.2.-condition)
   4. check value [formats](syntax/properties.md#3.8.3.-format) of string nodes
5) return final list of value nodes

## 6. Error Handling

Errors are reported if any step fails.
Possible error types include:

- runtime error
- calculator exception
- measurement exception
- unit system exception
- conversion exception
- atom parsing exception
- dimension map exception

## 7. Versioning

This document specifies version 0.1 of the DIPL language.

Language versioning is independent of any particular implementation. Each implementation MUST explicitly declare the version of the DIPL language it supports.

Future versions of the language MAY introduce extensions or modifications. Backward compatibility SHOULD be preserved where feasible.

## 8. Reference Implementation

A reference implementation of the DIPL language is provided as part of the SciNumTools v3 project.

The reference implementation includes:
- A C++ implementation
- Python bindings for integration and scripting use cases

**Project:**        SciNumTools v3

**Repository:**     [scinumtools3](https://github.com/vrtulka23/scinumtools3)

**Documentation:**  See project [documentation](https://vrtulka23.github.io/scinumtools3/) for usage and details

Independent implementations of the DIPL language (e.g., in Rust or Julia) are encouraged and welcome, and can be added to this list on request.

## 9. Future Development

In the next versions of DIPL we plan to introduce the following new concepts:

- improve expressions
- node containers, such as lists of nodes and associative node dictionaries
- container item schemas
- node templating

## 10. Authors and Contributors

- Ondrej Pego Jaura — Original author
