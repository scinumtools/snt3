---
title: "Dimensional Input Parameter Language"
subtitle: "DIPL Specification for SciNumTools v3"
author: "Ondrej Pego Jaura"
version: "0.1.0"
date: "September 2026"
titlepage: true
---

# DIPL - Language Specification

## Introduction

DIPL (Dimensional Input Parameter Language) is a domain-specific language for
describing input parameters used by scientific simulations and numerical
software.

Conventional configuration formats such as JSON, YAML, and INI are well suited
for representing general-purpose data, but provide little support for concepts
that are common in scientific computing. In particular, physical units,
dimensional consistency, numerical types, parameter dependencies, and
validation rules typically have to be implemented separately by the consuming
application.

DIPL addresses these requirements at the language level. It provides a
compact notation in which parameter values, units, structure, relationships,
and validation rules can be expressed together and subsequently evaluated by
a DIPL implementation.

The remainder of this specification defines the syntax and semantics of the
language and describes the constructs available for building DIPL documents.

## Language Overview

DIPL (Dimensional Input Parameter Language) is a declarative, strongly typed domain-specific language for defining structured scientific parameters, constraints, and relationships in a human-readable format.

A DIPL document consists of parameter declarations organized into hierarchical blocks through indentation. Each parameter combines:

* a name
* an explicit type
* an optional shape or structure
* a value expression
* an optional unit annotation
* optional properties for metadata, validation, and constraints

```DIPL
velocity float32[1,2:] = [[23.45, 23e-34, 45.1]] SI_km/s
    !condition ({?} < {?cfl_limit})

burst_energy float64 = 2.34e5 US_btu
```

A DIPL document is more than a collection of static values. 
It represents a structured and semantically validated configuration graph in which parameters, expressions, references, units, and constraints work together.

At a conceptual level:

* **Nodes** define the values and hierarchical structure of the configuration.
* **References and expressions** establish dependencies between parameters and allow values to be derived from other values.
* **Units** provide dimensional information and ensure physical consistency during evaluation.
* **Properties** attach metadata and declarative constraints to the configuration.
* **Schemas** define expected structures and provide a basis for validating complete configurations.

This design allows DIPL to represent not only data, but also the semantic relationships and domain-specific rules governing that data. 
The resulting document can be interpreted both as **data** and as an **executable configuration model**: values can be evaluated, dependencies resolved, and semantic constraints validated by the DIPL interpreter.

### Key Characteristics

**Strong typing**
All parameters declare explicit types (e.g., `bool`, `uint32`, `float64`, `str`), enabling predictable behavior, type checking, and validation.

**Units-aware computation**
Values may include physical units, which are automatically normalized and validated during evaluation, allowing dimensional consistency to be maintained throughout computations.

**Declarative constraints and metadata**
Properties such as `!condition` define constraints that must be satisfied for a valid document, while metadata such as `?author` provides descriptive information about individual nodes.

**Hierarchical organization**
An indentation-based structure allows related parameters to be organized into groups, maps, and lists without additional syntactic overhead.

**Referential expressions**
Parameters can reference other values within the document, allowing expressions to be built from existing data and making dependencies between parameters explicit.

**Derived parameters**
Parameters can be defined in terms of other parameters, allowing simple dependency graphs to be expressed directly within the language.

**External references**
Values and nodes can reference data from external sources, allowing DIPL documents to incorporate information without duplicating it locally.

**Composable schemas**
Schemas provide a mechanism for describing the expected structure and constraints of data, supporting consistent definitions and validation across documents.

## Language Syntax

The following chapters describe the syntax of the Dimensional Input Parameter Language. 
They introduce the individual language constructs used to define, represent, reference, and process scientific input data.

The syntax is presented from the fundamental building blocks of the language towards more advanced constructs. 
The chapters cover nodes and data types, values and units, references and expressions, as well as functions, properties, conditions, and schemas.

The following topics are covered:

* [Nodes](syntax/nodes.md) — the fundamental building blocks of a DIPL document.
* [Data Types](syntax/datatypes.md) — the types available for representing data.
* [Values](syntax/values.md) — how values are represented and assigned.
* [References](syntax/references.md) — how data can refer to other nodes and sources.
* [Expressions](syntax/expressions.md) — expressions for deriving and transforming values.
* [Units](syntax/units.md) — dimensional quantities and their units.
* [Functions](syntax/functions.md) — functions available for processing and transforming data.
* [Properties](syntax/properties.md) — metadata and properties associated with nodes.
* [Conditions](syntax/conditions.md) — conditional definitions and constraints.
* [Schemas](syntax/schemas.md) — schemas for defining and validating structured data.

## File Format

The source file MUST have an extension of either `.dip` or `.dipl`.
Nodes with the smallest [indentations](syntax/nodes.md#definition) in the file should be considered as the root nodes with an indentation level of 0.

## Execution Model

### Normative Execution Model

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

### Implementation Notes (Non-Normative)

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

## Error Handling

The following conditions MUST result in evaluation failure:

- unresolved source or node reference
- cyapic dependencies
- invalid query path
- type mismatch
- unit incompatibility
- expression evaluation failure

Errors MUST be deterministic and MUST NOT be ignored.

## Versioning

Language versioning is independent of any particular implementation. 
Each implementation MUST explicitly declare the version of the DIPL language it supports.

Future versions of the language MAY introduce extensions or modifications. 
Backward compatibility SHOULD be preserved where feasible.

## Reference Implementation

A reference implementation of the DIPL language is provided as part of the SciNumTools v3 project.

The reference implementation includes:
- A C++ implementation
- Python bindings for integration and scripting use cases

**Project:**        SciNumTools v3  
**Repository:**     <https://github.com/vrtulka23/scinumtools3>  
**Documentation:**  <https://vrtulka23.github.io/scinumtools3/>  
**Issues:**         <https://github.com/vrtulka23/scinumtools3/issues>  

Independent implementations of the DIPL language (e.g., in Rust or Julia) are encouraged and welcome, and can be added to this list on request.

## Glossary

| Term               | Meaning                                                                    |
|--------------------|----------------------------------------------------------------------------|
| **Node**           | Fundamental element of a DIPL document                                     |
| **Value node**     | Node containing a typed value                                              |
| **Container node** | Node containing child nodes                                                |
| **Path**           | Fully-qualified node path in the hierarchy                                 |
| **Group**          | Named container whose children are addressed by name                       |
| **Map**            | Container whose children are addressed by key                              |
| **List**           | Container whose children are addressed by index                            |
| **Schema**         | Reusable structural contract for nodes                                     |
| **Definition**     | Statement creating a node and assigning its value                          |
| **Declaration**    | Statement creating a typed node without a value                            |
| **Modification**   | Statement assigning a value to an existing node                            |
| **Source**         | Named external DIPL/text domain                                            |
| **Reference**      | Expression identifying existing nodes or external content                  |
| **Injection**      | Use of a reference as the value/content of a node                          |
| **Import**         | An operation that inserts referenced nodes into the current node hierarchy |
| **Injection**      | An operation that injects value to a node from local, or remote sources    |
| **Unit**           | Named physical or dimensional quantity                                     |
| **Expression**     | Construct that evaluates to a value                                        |
| **Property**       | Information attached to a node                                             |
| **Condition**      | Boolean validation rule                                                    |
| **Metadata**       | Non-semantic descriptive information attached to a node                    |
