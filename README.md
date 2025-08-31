# **SciNumTools v3**  
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
`scinumtools3` is a C++ library that helps scientists and engineers write safer, clearer numerical code by providing:

- **Expression Solver (EXS)** — parse and evaluate mathematical, logical, or custom expressions at runtime.  
- **Physical Units & Quantities (PUQ)** — represent values with units and perform unit-aware arithmetic.  
- **Dimensional Input Parameters (DIP)** — declare and validate parameter inputs with type & unit checks.

This project is the C++ counterpart to the original Python [scinumtools](https://github.com/vrtulka23/scinumtools/tree/main), focused on performance and strong typing.

---

## Features
- Expression parsing & evaluation  
- Strong SI unit support (units, prefixes, conversions, other unit systems e.g. CGS, AU)  
- Compile-time / runtime dimensional checks for parameters  
- Small, easy-to-integrate headers and CMake-friendly build  
- Unit tests included (GoogleTest)

---

## Installation

### Using CMake (recommended)
```bash
git clone https://github.com/vrtulka23/scinumtools3.git
cd scinumtools3
# using custom shell script
./setup.sh -b -t -i      # build, test and install
# or manually
cmake -B build
cd build
make -j $(getconf _NPROCESSORS_ONLN)
# optionally:
ctest
```

Add to your CMake project:

### Option 1: Add as a subdirectory
If you have cloned the repository inside your project:
```cmake
add_subdirectory(path/to/scinumtools3)
target_link_libraries(your_project PRIVATE snt-all)
```

---

## Quick Example

```cpp
#include <iostream>
#include <snt/exs/expression_solver.hpp>
#include <snt/puq/units.hpp>
#include <snt/dip/dip.hpp>

using namespace scinumtools;

int main() {
    // Expression solver
    exs::ExpressionSolver solver;
    double result = solver.evaluate("3 * (2 + 5)");
    std::cout << "Expression result: " << result << std::endl;

    // Quantities with units
    puq::Quantity length = 10.0 * units::meter;
    puq::Quantity time = 2.0 * units::second;
    puq::Quantity speed = length / time;
    std::cout << "Speed: " << speed.value() << " m/s" << std::endl;

    // Dimensional Input Parameter
    d = dip::DIP();
    std::cout << "Pressure: " << pressure.value() << " Pa" << std::endl;

    return 0;
}
```

---

## Documentation

API reference and guides are available in the `docs/` directory.

To generate Doxigen + breathe + Sphinx documentation (if configured):

```bash
./docs/setup.sh -b
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
   ./setup.sh -b -t
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

