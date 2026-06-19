# SciNumTools3 for Python

[![PyPI version](https://badge.fury.io/py/scinumtools3.svg)](https://badge.fury.io/py/scinumtools3)
![Python](https://img.shields.io/badge/python-3.9%2B-blue.svg)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

**Unit-safe quantities, runtime expressions, and validated scientific parameters for Python.**

SciNumTools3 helps eliminate hidden assumptions in scientific software by providing:

* ✅ Physical quantities with units
* ✅ Automatic unit conversion
* ✅ Runtime expression evaluation
* ✅ Validated input parameters
* ✅ Structured scientific configuration files
* ✅ Consistent parameter handling across simulation and analysis workflows

## Relationship to the C++ Core

`scinumtools3` is the official Python binding for the SciNumTools3 C++ project.

The Python package exposes the same core concepts used by the C++ implementation, including:

- Physical Units & Quantities (PUQ)
- Dimensional Input Parameters (DIP)
- Runtime expression evaluation
- Unit-aware validation and conversion

This enables consistent parameter definitions and unit handling across Python analysis workflows and C++ simulation codebases.

### C++ Project

The underlying C++ implementation is available on GitHub:

https://github.com/vrtulka23/scinumtools3

The C++ project serves as the reference implementation and provides high-performance support for scientific computing, numerical simulations, and HPC workflows.

---

## Installation

Install directly from PyPI:

```bash
pip install scinumtools3
```

PyPI package:

https://pypi.org/project/scinumtools3/

---

## Why SciNumTools3?

Scientific workflows frequently suffer from:

* inconsistent unit handling
* manual conversion mistakes
* duplicated parameter definitions
* missing validation
* configuration drift between simulation and analysis code

SciNumTools3 provides a unified framework for defining, validating, and evaluating scientific parameters while preserving physical units throughout calculations.

---

## Core Features

### Physical Quantities

Create quantities with physical units and perform automatic conversions:

```python
from scinumtools3.puq import Quantity

length = Quantity("1*m")
length = length.convert("km")

print(length)
# 1e-3*km
```

---

### Unit-Aware Arithmetic

```python
from scinumtools3.puq import Quantity

distance = Quantity("1000*m")
time = Quantity("100*s")

velocity = distance / time

print(velocity)
# 10*m*s-1
```

---

### Validated Parameters (DIP)

Define scientific parameters with units, constraints, and dependencies.

```python
from scinumtools3.dip import DIP

dip = DIP()

dip.add_string("temperature float = 300 K")
dip.add_string("pressure float = 1 atm")

env = dip.parse()
```

Example parameter specification:

```text
simulation

  timestep float = 0.5 fs
    !condition ({?} > 0.0 fs)

  temperature float = 300 K
    !condition ({?} > 0 K)

  pressure float = 1 atm

  steps int = 1000000
    !condition ({?} > 1)

  duration float = ( {?simulation.timestep} * {?simulation.steps} )

  ensemble string = "NPT"
    !options ["NVE", "NVT", "NPT"]
```

---

## Domain-Specific Languages

SciNumTools3 is built around two scientific DSLs.

### PUEL — Physical Units Expression Language

A concise language for representing physical quantities, units, dimensions, uncertainties, and arrays.

Examples:

```text
ESU_erg
m2*kg*s-2
kg2*ms3:2*cm
1.346591(30)e27*kg
[2,3,4,5]*km
```

PUEL powers:

* quantity parsing
* dimensional analysis
* unit conversions
* unit-aware calculations

---

### DIPL — Dimensional Input Parameter Language

A declarative language for defining scientific parameters with:

* types
* units
* constraints
* dependencies
* validation rules

This allows scientific applications to maintain a single source of truth for configuration and input validation.

---

## Quick Example

```python
from scinumtools3.puq import Quantity
from scinumtools3.dip import DIP

# Unit conversion
length = Quantity("1*m")
length = length.convert("km")

print(length)
# 1e-3*km

# Validated parameters
dip = DIP()

dip.add_string("foo int m")
dip.add_string("foo = 3 km")

env = dip.parse()

print(env.nodes[0])
# 3000 m
```

---

## Documentation

Full documentation, API reference, language specifications, and examples:

https://vrtulka23.github.io/scinumtools3/

Documentation includes:

* Python API reference
* PUEL specification
* DIPL specification
* Tutorials and examples

---

## Typical Use Cases

* Scientific computing
* Physics simulations
* Engineering applications
* Data analysis pipelines
* Research software
* Hybrid Python/C++ workflows
* Unit-safe parameter management

---

## Contributing

Contributions are welcome.

```bash
git clone https://github.com/vrtulka23/scinumtools3.git
cd scinumtools3
```

Please open an issue or pull request for bug fixes, improvements, or new features.

---

## License

SciNumTools3 is released under the MIT License.

See the `LICENSE` file for details.

---

## Links

* PyPI: https://pypi.org/project/scinumtools3/
* Documentation: https://vrtulka23.github.io/scinumtools3/
* GitHub: https://github.com/vrtulka23/scinumtools3
* Issues: https://github.com/vrtulka23/scinumtools3/issues
