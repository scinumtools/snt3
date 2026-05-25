# **Scientific Numerical Tools v3 (SciNumTools3, SNT)**  
*``SciNumTools3`` is a C++ library for unit-safe, validated parameters—eliminating hidden assumptions in scientific code.*

[![C/C++ CI](https://github.com/vrtulka23/scinumtools3/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/vrtulka23/scinumtools3/actions/workflows/c-cpp.yml)
[![Linux Build](https://img.shields.io/github/actions/workflow/status/vrtulka23/scinumtools3/c-cpp.yml?label=Linux%20build)](https://github.com/vrtulka23/scinumtools3/actions/workflows/build-linux.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
![GitHub release](https://img.shields.io/github/v/release/vrtulka23/scinumtools3?include_prereleases)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg?logo=c%2B%2B&logoColor=white)
![GCC](https://img.shields.io/badge/gcc-%3E%3D11-blue)
![Clang](https://img.shields.io/badge/clang-%3E%3D20-blue)

---

## Overview

Scientific software often suffers from inconsistent parameter definitions, ad hoc unit handling, and missing validation. These issues become particularly problematic in workflows that span multiple components or languages.

``SciNumTools3`` (or ``SNT``) addresses this by providing a unified system that combines:

- Runtime expression evaluation → define relationships without recompilation  
- Unit-aware quantities → prevent scaling and conversion errors  
- Validated input parameters → detect invalid configurations early  
- Cross-language consistency → shared parameter logic between C++, Python and CLI  
- Lightweight integration → header-based design with CMake support 

into a single configuration layer that can be used consistently across workflows.

``SNT`` is a C++ library, with optional Python bindings, designed for safer and more transparent numerical computation. 
It is built around four core components:

**Value Layer** (VAL) — a unified, typed multidimensional data model for booleans, numeric values, and strings, forming the core runtime representation shared across all components  

**Expression Solver** (EXS) — a generic, extensible evaluation framework capable of operating on arbitrary data types defined as atoms; its behavior is fully determined by the set of atom types, operators/functions, and evaluation rules (precedence and semantics), making it adaptable to domain-specific evaluation systems  

**Physical Units & Quantities** (PUQ) (built on VAL + EXS) — extends the value system with physical units, enabling unit-aware arithmetic, dimensional consistency, and automatic conversions through EXS-based evaluation  

**Dimensional Input Parameters** (DIP) (built on VAL + EXS + PUQ) — a declarative parameter definition layer that enforces types, units, constraints, and structure, while delegating all numerical, logical, and unit-aware expression evaluation to EXS via PUQ  

Together, these components establish a validated, unit-aware configuration framework that can be consistently shared across heterogeneous environments, including C++ simulations, Python-based analysis workflows, and Bash-driven processing pipelines.

This design enables:

- a single source of truth for parameters
- consistent interpretation of units and values across codebases
- early detection of invalid or inconsistent inputs

As a result, discrepancies between simulation and analysis pipelines are significantly reduced.

This project is the C++ counterpart to the original Python [SciNumTools v2](https://github.com/vrtulka23/scinumtools/tree/main), with a focus on performance, static typing, and integration into high-performance computing workflows.

## Why use SNT?

Compared to alternatives:

- Boost.Units → compile-time only, no runtime expressions
- pint → Python-only, no C++ integration
- ad hoc configs → no validation, no unit safety

``SNT`` combines:
- runtime expressions → dynamic configs without recompilation
- unit safety → prevents silent scaling errors
- validated parameters → fail fast instead of corrupting simulations

in a single system.

## Target Use Cases

- scientific simulations (C/C++, HPC)
- physics / engineering pipelines
- parameter-heavy workflows with unit safety requirements
- hybrid C++ / Python environments

---

## Installation

### Download and install

1) Manually

   ```bash
   # download repository
   git clone https://github.com/vrtulka23/scinumtools3.git
   cd scinumtools3
   
   # compile
   cmake -B build
   cd build
   make
   
   # run tests
   ctest
   
   # install
   sudo make install
   ```

2) Using setup script

   ```bash
   sudo ./setup.sh -b -t -i  # build, run tests, install
   ```

### Link `SciNumTools3` in your `CMAKE` project

1) Find the package

   ```bash
   # find the `SNT` package
   find_package(snt REQUIRED)

   # link to your executable
   add_executable(${EXEC_NAME} ${SOURCE_FILES})
   target_link_libraries(${EXEC_NAME} PRIVATE snt-exs snt-puq snt-dip)
   ```

## Quick Example

### Domain Specific Languages

`SciNumTools` is built around two domain-specific languages that form the foundation of the framework and enable consistent handling of scientific notation and dimensional analysis. The first language, [PUEL](docs/puel/specification.md), defines a formal notation system for representing physical units and quantities in a precise and machine-readable way. The second language, [DIPL](docs/dipl/specification.md), provides a textual specification format for defining dimensional parameters and their relationships. Together, these languages establish the core abstraction layer of `SciNumTools`, allowing scientific data, units, and dimensional constraints to be expressed in a structured, interoperable, and extensible manner.

#### Physical Units Expression Language - PUEL

Expressions of physical units must account for several important aspects, including the underlying unit system (such as SI, CGS, Atomic Units, or US Customary Units), unit prefixes and scaling factors (such as `kg`, `mm`, or `MJ`), the associated numerical values (whether scalar values or arrays), uncertainties arising from measurements, the relationship between base units and physical dimensions, as well as support for integer and fractional exponents of units.

`PUEL` provides a minimal, coherent and extensible notation that integrates all of these concerns into a unified representation. 

``` puel
# General form
<SYSTEM>_[<VALUE>]*<UNIT><EXPONENT>

# Examples
ESU_erg              # sefining erg in ESU unit system
m2*kg*s-2            # definition of complex units
kg2*ms3:2*cm         # fractional exponents
1.346591(30)e27*kg   # uncertainties in measurements
[2, 3, 4, 5]*km      # arrays of values
```

Building upon this specification, the `PUQ` module of `SciNumTools` implements parsing, dimensional analysis, arithmetic operations, and unit conversion both within a single unit system and across different systems.

#### Dimensional Input Parameter Language - DIPL

The definition of input parameters for scientific and engineering software involves several recurring requirements that are common across many numerical codes. High-performance applications often require strongly typed parameters with explicitly defined numerical precision. Numerical values frequently carry associated physical units, and many parameters consist not only of scalar values, but also arrays, matrices, or tabulated datasets.

```dipl
family

  father float = 184 cm

  mother float = 161 cm

  children int = 2
    !constant

    son float = 112 cm

    daughter float = 98 cm

  members float = ({?family.children} + 2)
```

In addition, parameters commonly require validation rules, numerical constraints, admissible ranges, or configurable options that govern their behavior and interpretation. Parameter definitions are often interdependent as well, where the validity, availability, or meaning of one setting depends on the values of others. `DIPL` provides a coherent and extensible framework for expressing these definitions, relationships, and constraints in a structured, machine-readable, and implementation-independent form.

### Example of Use

Below is a quick example how to use the core functionality of `scinumtools3`.
For more examples and patterns please look into the ``tests``, ``exec``, ``apps`` and ``bindings`` folders.

#### with C++

```cpp
#include <snt/exs/atom.h>
#include <snt/exs/solver.h>
#include <snt/val/values_array.h>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>
#include <snt/dip/nodes/node_value.h>
#include <iostream>

using namespace snt;

int main() {

  exs::Solver<exs::Atom> solver;
  exs::Atom atom = solver.solve("23 * 34.5 + 4");
  std::cout << atom.to_string() << std::endl;
  // 797.5

  val::ArrayValue<double> value({1.23, 4.56e7});
  std::cout << value.to_string() << std::endl;
  // [1.23, 4.56e7]

  puq::Quantity length("1*m");
  length = length.convert("km");
  std::cout << length.to_string() << std::endl;
  // 1e-3*km

  dip::DIP d;
  d.add_string("foo int m");
  d.add_string("foo = 3 km");
  dip::Environment env = d.parse();
  dip::ValueNode::PointerType vnode = env.nodes.at(0);
  std::cout << vnode->value->to_string() << std::endl;
  // 3000
}
```

#### with Python 

``` python
from scinumtools3.puq import Quantity
from scinumtools3.dip import DIP, Environment

length = Quantity("1*m")
length = length.convert("km");
print(length) 
# 1e-3*km

dip = DIP()
dip.add_string("foo int m");
dip.add_string("foo = 3 km");
env = dip.parse();
print(env.nodes[0])
# 3000 m
```

#### with CLI (e.g. BASH)

``` bash
snt puq convert "12*statA" "A" -s ESU -S SI -Q "I"
# 4.00277e-9*A

snt dip parse \
    -f parameters.dip \
    -r "?family.father" \
    --print
# father = 184 cm
```

---

## Documentation

The ``docs/`` directory contains the complete API reference and user guides for the project.
The online documentation for the C++ reference implementation, including Python bindings and the CLI, is available [here](https://vrtulka23.github.io/scinumtools3/).

It also provides detailed specifications for the DIPL and PUEL domain-specific languages:

[DIPL](docs/dipl/specification.md) is used to define validated, structured input parameters.  
[PUEL](docs/puel/specification.md) defines a syntax for unit-aware expressions and calculations.  

---

## Contributing

Contributions are welcome — please follow these guidelines:

1. Fork the repo and create a feature branch:

   ```bash
   git clone https://github.com/vrtulka23/scinumtools3.git
   git checkout -b feature/my-feature
   ```
2. Follow the coding style (`.clang-format`) and use modern C++ (C++17+).
3. Add unit tests for new features or bug fixes (see `tests/`).
4. Build and run tests locally:

   ```bash
   ./setup.sh -b -t   # build, run tests
   ```
5. Open a Pull Request with a clear description and link to any related issues.

See [CONTRIBUTING.md](https://github.com/vrtulka23/scinumtools3/blob/main/CONTRIBUTING.md) for full instructions.

## License

This project is licensed under the **MIT License**. See the [LICENSE](https://github.com/vrtulka23/scinumtools3/blob/main/LICENSE) file for full text.

## Contact / Issues

Found a bug or have a feature request? Open an issue at:
[https://github.com/vrtulka23/scinumtools3/issues](https://github.com/vrtulka23/scinumtools3/issues)

