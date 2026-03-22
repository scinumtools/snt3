# DIPL - Language Specification

**Version:** v1.0  
**Status:** Stable (Implementation-defined semantics)  
**Date:** 2026-03-17  
**Reference Implementation:** SciNumTools v3

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

DIPL is a declarative, strongly-typed domain-specific language for defining structured scientific parameters, constraints, and relationships in a human-readable format.

A DIPL document consists of parameter declarations organized into hierarchical blocks via indentation. Each parameter combines:

- a name
- an explicit type
- an optional shape or structure
- a value expression
- an optional unit annotation
- optional properties for validation and constraints

This design allows DIPL to represent not only data, but also semantic rules and domain-specific logic in a single, unified format.

### 2.1. Structure

Parameters are defined line-by-line. Indentation introduces hierarchical grouping, enabling nested structures without additional syntax.

```DIPL
parameter_name type = value [unit]
    !property (expression)
```

- Indentation defines scope and hierarchy
- Properties (prefixed with ``!``) attach constraints or metadata to the preceding parameter
- Expressions may reference other parameters and include numerical or logical operations

### 2.2. Example

```DIPL
velocity float32[1,2:] = [[23.45, 23e-34, 45.1]] SI_km/s
    !condition ("{?} < {?cfl_limit}")

burst_energy float64 = 2.34e5 US_btu
```

### 2.3. Key Characteristics

**Strong typing**  
All parameters declare explicit types (e.g., float32, int, arrays, tables), enabling predictable behavior and validation.

**Units-aware computation**  
Values may include physical units, which are automatically normalized and validated during evaluation.

**Declarative constraints**  
Properties such as ``!condition`` define validation rules that must hold for a valid document.

**Hierarchical organization**  
Indentation-based structure allows grouping of related parameters without additional syntactic overhead.

**Referential expressions**  
Parameters can reference other values within the document, enabling dependency-aware configurations.

### 2.4. Conceptual Model

A DIPL file is not just static data; it represents a validated configuration graph:

- Parameters define values and structure
- Expressions establish dependencies between parameters
- Units ensure physical consistency
- Properties enforce constraints

The result is a configuration that is both machine-evaluable and semantically validated.

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

### 5.1 Normative Execution Model

Evaluation of a DIPL document MUST proceed in the following stages, in order:

1. **Parsing**  
   The input MUST be parsed into a structured representation of nodes.
   This includes processing indentation, node types, and structural relationships.
2. **Dependency Resolution**  
   All references MUST be resolved into a directed acyclic graph (DAG) of node dependencies.
   - Unresolved sources or node references MUST result in an error.
   - Cyclic dependencies MUST result in an error.
3. **Reference Evaluation**  
   All reference expressions (e.g. ``{<source>?<query>}``, ``{?<query>}``) MUST be resolved to their corresponding values or node sets.
   Remote sources MUST be processed independently before their results are used.
4. **Expression and Value Evaluation**  
   All node values MUST be evaluated, including:
   - scalar values
   - arrays and structured values
   - expressions involving references
   If evaluation fails (e.g. invalid operations or unresolved values), evaluation MUST fail.
5. **Unit Normalization**  
   All evaluated values MUST be converted to the canonical unit representation defined in the Units specification.
   Unit incompatibility MUST result in an error.
6. **Condition Evaluation**  
   All conditions MUST be evaluated using the normalized value of the node.
   The ``{?}`` self-reference refers to this value.
   - If a condition evaluates to false, the node MUST be considered invalid.
   - If condition evaluation fails, evaluation MUST fail.
7. **Validation**  
   All validation rules MUST be applied, including:
   - option constraints
   - value format checks
   - structural constraints  
   
   Any violation MUST result in an error.

All conforming implementations MUST produce identical results for identical inputs and sources.
If any stage fails, evaluation MUST terminate with an error.

### 5.2 Implementation Notes (Non-Normative)

A DIPL processor may implement the above stages using steps such as:

- parsing code lines into nodes
- combining multiline string blocks into single strings (e.g. text wrapped in ``"""``)
- replacing special symbols with temporary markers (e.g. ``\n``, ``\'``, ``\"``) and restoring them later
- determining node types (e.g. group, value, case)
- constructing node hierarchy and full names (e.g. ``foo.bar.baz``)
- assigning property nodes to preceding value nodes
- validating indentation and structural consistency
- resolving references and expanding nodes
- evaluating expressions and parsing values (e.g. scalars like ``34e+3``, arrays like ``[[1,3],[4,5]]``)
- parsing and applying unit expressions (e.g. ``kg/s``)
- applying modifications to existing nodes
- evaluating conditional branches and case expressions
- validating nodes against constraints (options, conditions, formats)

These steps are provided for guidance only. Implementations MAY use different internal strategies, provided the observable behavior conforms to the normative execution model defined above.

## 6. Error Handling

The following conditions MUST result in evaluation failure:

- unresolved source or node reference
- cyclic dependencies
- invalid query path
- type mismatch
- unit incompatibility
- expression evaluation failure

Errors MUST be deterministic and MUST NOT be ignored.

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
