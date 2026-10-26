---
title: "Dimensional Input Parameter Language"
subtitle: "DIPL Language Specification"
author: "Ondrej Pego Jaura [![ORCID iD](../source/_static/orcid-id.png)](https://orcid.org/0000-0002-5391-3714)"
version: "1.1.0"
date: "October 2026"
doi: "10.5281/zenodo.23376301"
keywords:
  - domain-specific language
  - scientific computing
  - configuration language
  - input parameters
  - physical units
titlepage: true
---

# DIPL Language Specification — Version 1.1.0

**DOI:** [10.5281/zenodo.23376301](https://doi.org/10.5281/zenodo.23376301)

## Abstract

DIPL (Dimensional Input Parameter Language) is a declarative language for
describing structured input parameters for scientific and numerical software.
It brings parameter values, data types, physical units, constraints, metadata,
and relationships into one human-readable format. This specification defines
DIPL's syntax, type system, expression and unit semantics, schemas, validation,
and execution model. It is intended for language users and implementers, and
serves as the reference for the SNT implementation. The document includes a
formal grammar and examples of the language's main constructs. DIPL aims to make
scientific input definitions easier to inspect, validate, reuse, and exchange
while preserving the dimensional and structural information that applications
need.

**Keywords:** domain-specific language; scientific computing; configuration
language; input parameters; physical units

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
    !condition ({.} < {?cfl_limit})

burst_energy float64 = 2.34e5 US_btu
```

A DIPL document is more than a collection of static values. 
It represents a structured and semantically validated configuration graph in which parameters, expressions, references, units, and constraints work together.

At a conceptual level:

* **Nodes** define the values and hierarchical structure of the configuration.
* **References and expressions** establish dependencies between parameters and allow values to be derived from other values.
* **Links** expose a canonical node through another read-only path without copying its data.
* **Units** provide dimensional information and ensure physical consistency during evaluation.
* **Properties** attach metadata and declarative constraints to the configuration.
* **Schemas** define expected structures and provide a basis for validating complete configurations.
* **Overrides** replace values in an existing parameter model while preserving its declarations and constraints.

This design allows DIPL to represent not only data, but also the semantic relationships and domain-specific rules governing that data. 
The resulting document can be interpreted both as **data** and as an **executable configuration model**: values can be evaluated, dependencies resolved, and semantic constraints validated by the DIPL interpreter.

### Key Characteristics

**Strong typing**
All parameters declare explicit types (e.g., `bool`, `uint32`, `float64`, `str`), enabling predictable behavior, type checking, and validation.

**Units-aware computation**
Values may include physical units, which are automatically normalized and validated during evaluation, allowing dimensional consistency to be maintained throughout computations.

**Declarative constraints and metadata**
Properties such as `!condition` define constraints that must be satisfied for a valid document, while metadata such as `?authors` provides descriptive information about individual nodes.

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

**Guided setup annotations**
The optional ``!setup`` directive marks typed values for interactive input by
supporting tools. Input controls can be inferred from the declared type,
``!options``, ``!labels``, units, and existing validation properties.

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
* [Schemas](syntax/schemas.md) — schemas for defining and validating structured data, including required metadata constraints.
* [Overrides](syntax/overrides.md) — value replacements applied before normal evaluation.

## File Format

The source file MUST have an extension of either `.dip` or `.dipl`.
Nodes with the smallest [indentations](syntax/nodes.md#definition) in the file should be considered as the root nodes with an indentation level of 0.

## Execution Model

### Normative Execution Model

Evaluation of a DIPL document MUST proceed in the following stages, in order:

1. **Parsing**  
   The input MUST be parsed into a structured representation of nodes.
   This includes processing indentation, node types, and structural relationships.
   All top-level override regions MUST be collected at this stage, before
   normal node evaluation. Duplicate override targets MUST result in an error.
2. **Dependency Resolution**  
   Value references MUST be resolved into a directed acyclic graph (DAG) of node dependencies.
   - An unresolved source or ordinary value reference MUST result in an error.
     The `def` and `ndef` operators test whether a node has a non-`none` value
     in an already-resolved domain at the point of evaluation. A missing node,
     a declaration without an assigned value, or an explicit `none` value
     yields `false` for `def` and `true` for `ndef`. These operators do not
     suppress errors resolving the source domain.
   - Cyclic dependencies MUST result in an error.
3. **Reference Evaluation**  
   All ordinary reference expressions (e.g. ``{<source>?<path>}``,
   ``{?<path>}``) MUST be resolved to their corresponding values or node sets.
   `def` and `ndef` evaluate the referenced path's definition state as
   described above.
   Remote sources MUST be processed independently before their results are used.
4. **Expression and Value Evaluation**  
   All node values MUST be evaluated, including:
   
   - scalar values
   - arrays and structured values
   - expressions involving references
   
   For an overridden node, only its replacement value MUST be evaluated, at
   the target declaration. Its dependencies MUST already be available then.
   The original value MUST NOT be evaluated. Subsequent dependents and
   conditional definitions MUST use the replacement value. These rules are
   defined in the [Overrides](syntax/overrides.md) chapter.
   If evaluation fails (e.g. invalid operations or unresolved values), evaluation MUST fail.
5. **Unit Normalization**  
   All evaluated values MUST be converted to the canonical unit representation defined in the Units specification.
   Unit incompatibility MUST result in an error.
6. **Condition Evaluation**  
   All conditions MUST be evaluated using the normalized value of the node.
   The ``{.}`` self-reference refers to this value.
   - If a condition evaluates to false, the node MUST be considered invalid.
   - If condition evaluation fails, evaluation MUST fail.
7. **Validation**  
   All validation rules MUST be applied, including:
   - option constraints
   - value format checks
   - structural constraints
   - schema metadata constraints against effective metadata after schema defaults, instance metadata, and supported overrides have been applied. `required` MUST accept an explicitly present empty value; `nonempty` MUST reject empty or whitespace-only strings and empty or blank list entries.
   
   Any violation MUST result in an error.

Except for host-defined functions, conforming implementations MUST produce identical results for identical inputs and sources.
Functions in DIPL are exception from this rule, because they are user defined and depend on the particular implementation and use case.
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
- cyclic dependencies
- invalid query path
- type mismatch
- unit incompatibility
- expression evaluation failure
- duplicate, invalid, or unresolved override target
- a violated schema metadata constraint

Errors MUST be deterministic and MUST NOT be ignored.

## Versioning

Language versioning is independent of any particular implementation. 
Each implementation MUST explicitly declare the version of the DIPL language it supports.

Future versions of the language MAY introduce extensions or modifications. 
Backward compatibility SHOULD be preserved where feasible.

Version 1.1.0 adds the valueless ``!setup`` property. Documents that do not use
this property retain their Version 1.0.0 behavior.

## Reference Implementation

A reference implementation of the DIPL language is provided as part of the SciNumTools v3 project.

The reference implementation includes:

- A C++ implementation
- Python bindings for integration and scripting use cases

**Project:**        SciNumTools v3  
**Repository:**     <https://github.com/scinumtools/snt3>  
**Documentation:**  <https://scinumtools.github.io/snt3/>  
**Issues:**         <https://github.com/scinumtools/snt3/issues>  

Independent implementations of the DIPL language (e.g., in Rust or Julia) are encouraged and welcome, and can be added to this list on request.

## License

This specification is distributed under the MIT License. The license text is
available in the [SciNumTools v3 repository](https://github.com/scinumtools/snt3/blob/main/LICENSE).

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
| **Override**       | Collected value replacement evaluated at an existing node's declaration    |
| **Source**         | Named external DIPL/text domain                                            |
| **Reference**      | Expression identifying existing nodes or external content                  |
| **Import**         | An operation that inserts referenced nodes into the current node hierarchy |
| **Injection**      | An operation that injects value to a node from local, or remote sources    |
| **Unit**           | Named physical or dimensional quantity                                     |
| **Expression**     | Construct that evaluates to a value                                        |
| **Property**       | Information attached to a node                                             |
| **Condition**      | Boolean validation rule                                                    |
| **Metadata**       | Non-semantic descriptive information attached to a node                    |
