# **Scientific Numerical Tools v3 (SNT)**  
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

``scinumtools3`` (or ``snt``) addresses this by providing a unified system that combines:

- unit-aware quantities
- runtime expression evaluation
- validated parameter definitions

into a single configuration layer that can be used consistently across workflows.

``snt`` is a C++ library, with optional Python bindings, designed for safer and more transparent numerical computation. 
It is built around three core components:

**Expression Solver** (EXS) — parses and evaluates mathematical, logical, and custom expressions at runtime  
**Physical Units & Quantities** (PUQ) — represents values with physical units and supports unit-aware arithmetic and conversions  
**Dimensional Input Parameters** (DIP) — defines and validates input parameters with explicit types and unit constraints  

Together, these components form a validated, unit-aware configuration system that can be shared across heterogeneous environments (e.g. C++ simulation code and Python analysis workflows).

This design enables:

- a single source of truth for parameters
- consistent interpretation of units and values across codebases
- early detection of invalid or inconsistent inputs

As a result, discrepancies between simulation and analysis pipelines are significantly reduced.

This project is the C++ counterpart to the original Python [scinumtools](https://github.com/vrtulka23/scinumtools/tree/main), with a focus on performance, static typing, and integration into high-performance computing workflows.

---

## Core Capabilities

- Expression parsing & evaluation  
- Strong SI unit support (units, prefixes, conversions, other unit systems e.g. CGS, AU)  
- Compile-time / runtime dimensional checks for parameters  
- Small, easy-to-integrate headers and CMake-friendly build  
- Unit tests included (GoogleTest)

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

### Link `scinumtools3` in your `CMAKE` project

1) Find the package

   ```bash
   # find the `SNT` package
   find_package(snt REQUIRED)

   # link to your executable
   add_executable(${EXEC_NAME} ${SOURCE_FILES})
   target_link_libraries(${EXEC_NAME} PRIVATE snt-exs snt-puq snt-dip)
   ```

---

## Quick Example

Below is a quick example how to use the core functionality of `scinumtools3`.
For more examples and patterns please look into the ``gtest`` and ``exec`` folders.

```cpp
#include <snt/exs.h>
#include <snt/val.h>
#include <snt/puq.h>
#include <snt/dip.h>
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

---

## Documentation

API reference and guides are available in the `docs/` directory.

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

---

## License

This project is licensed under the **MIT License**. See the [LICENSE](https://github.com/vrtulka23/scinumtools3/blob/main/LICENSE) file for full text.

---

## Contact / Issues

Found a bug or have a feature request? Open an issue at:
[https://github.com/vrtulka23/scinumtools3/issues](https://github.com/vrtulka23/scinumtools3/issues)

