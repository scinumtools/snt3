# DIPL Language Specification

Version: 0.1  
Status: Draft  
Project: SciNumTools

---

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

## Parameter Declaration

## Types

## Shape Specification

## Values

## Units and Dimensions

## Expressions

## References

## Metadata Tags
 
## Execution Model
 
## Error Handling

## Examples

## Versioning

## Reference Implementation