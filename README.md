# **Scientific Numerical Tools v3 (SciNumTools3, SNT)**  
*A modern C++ toolkit for scientific computations with units, expressions, and dimensional parameters.*

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
It is built around three core components:

**Expression Solver** (EXS) — parses and evaluates mathematical, logical, and custom expressions at runtime  
**Physical Units & Quantities** (PUQ) — represents values with physical units and supports unit-aware arithmetic and conversions  
**Dimensional Input Parameters** (DIP) — defines and validates input parameters with explicit types and unit constraints  

Together, these components establish a validated, unit-aware configuration framework that can be consistently shared across heterogeneous environments, including C++ simulations, Python-based analysis workflows, and Bash-driven processing pipelines.

This design enables:

- a single source of truth for parameters
- consistent interpretation of units and values across codebases
- early detection of invalid or inconsistent inputs

As a result, discrepancies between simulation and analysis pipelines are significantly reduced.

This project is the C++ counterpart to the original Python [scinumtools](https://github.com/vrtulka23/scinumtools/tree/main), with a focus on performance, static typing, and integration into high-performance computing workflows.

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

Below is a quick example how to use the core functionality of `SciNumTools3`.
For more examples and patterns please look into the ``tests`` and ``examples`` folders.

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

## Documentation

The `docs/` directory contains the full API reference and user guides for the project. It also includes detailed specifications for the DIPL and PUEL domain-specific languages. [DIPL](docs/dipl/specification.md) is used for defining validated, structured input parameters, while [PUEL](docs/puel/specification.md) provides a syntax for unit-aware expressions and calculations.

To generate Doxygen + breathe + Sphinx documentation:

* Using the setup script

  ```bash
  ./setup.sh -d   # build documentation
  ```
  
* Manually

  ```bash
  cd docs
  make html
  ```

See `docs/README.md` for more information.

---

## Contributing

Contributions are welcome — please follow these guidelines:

1. Fork the repo and create a feature branch:

   ```bash
   git clone https://github.com/vrtulka23/scinumtools3.git
   git checkout -b feature/my-feature
   ```
2. Follow the coding style (`.clang-format`) and use modern C++ (C++17+).
3. Add unit tests for new features or bug fixes (see `gtest/`).
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

