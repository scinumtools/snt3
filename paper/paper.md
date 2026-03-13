---
title: "DIPL: A Dimensional Input Parameter Language for Scientific Simulation Configuration"
tags:
  - C++
  - Python
  - scientific computing
  - configuration
  - units
authors:
  - name: Ondrej Pego Jaura
    affiliation: 1
affiliations:
  - name: Unaffiliated
    index: 1
date: 2026
bibliography: paper.bib
---

# Summary

DIPL (Dimensional Input Parameter Language) is a domain-specific language
for defining typed, unit-aware parameters used in scientific simulations and
numerical codes. The language allows users to specify parameters with
explicit dimensional units, array shape constraints, references to other
parameters, expressions, and validation rules. DIPL is implemented in the
SciNumTools framework and is designed to provide a robust alternative to
general-purpose configuration formats such as JSON or YAML when working with
dimensionally consistent scientific data.

The reference implementation of DIPL is provided in the SciNumTools library,
which offers parsing, unit conversion, expression evaluation, and validation
of simulation parameters. Python bindings are provided to facilitate
integration with scientific workflows.

# Statement of Need

Scientific simulation software often relies on configuration files to define
parameters controlling numerical models, physical constants, and simulation
settings. Common configuration formats such as :contentReference[oaicite:1]{index=1}
or :contentReference[oaicite:2]{index=2}
lack native support for dimensional units, parameter validation, and
structured references between parameters.

As a result, scientific codes frequently implement custom parsing logic to
handle units, validate parameters, and compute derived quantities. This
approach leads to duplicated infrastructure across projects and increases
the risk of configuration errors.

DIPL addresses these challenges by providing a language designed
specifically for scientific parameter specification. Key features include:

- explicit dimensional units
- typed parameters and storage types
- array shape constraints
- expressions referencing other parameters
- validation rules attached to parameters

These features enable simulation parameters to be defined in a declarative,
self-documenting format that can be validated before numerical computation
begins.

# Implementation

The reference implementation of DIPL is part of the SciNumTools library,
which is written in C++ and provides Python bindings for use in scientific
workflows. The implementation consists of several modular components:

- **DIP**: the parser and runtime system for DIPL parameter files
- **PUQ**: a physical quantity system providing dimensional units and unit
  conversions
- **EXS**: an expression solver for evaluating parameter expressions
- **VAL**: a dynamic value representation used for parameter storage

A typical DIPL parameter declaration may take the form:

```dipl
velocity float32[1,2:] = [[23.45, 23e-34, 45.1]] SI_km/s
    !condition (" {?} < {?cfl_limit} ")

burst_energy float64 = 2.34e5 US_btu
```

In this example, parameters specify storage type, array shape, numeric
values, and physical units. Validation conditions can be attached to ensure
constraints are satisfied before the simulation begins.

# Example Use

DIPL can be used to define parameters for numerical simulations in a concise
and physically meaningful way. For example:

```
distance float64 = 10 km
time float64 = 5 s
velocity float64 = (" {distance} / {time} ") km/s
```

The expression engine evaluates derived quantities while preserving unit
consistency.

# Availability

Source code: https://github.com/vrtulka23/scinumtools3

Documentation: https://vrtulka23.github.io/scinumtools3/

Acknowledgements

The author thanks colleagues and collaborators from previous research environments for discussions and inspiration related to scientific computing and simulation software.