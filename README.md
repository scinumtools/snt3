# **Scientific Numerical Tools v3 (SciNumTools3, SNT)**

*Unit-safe, strongly typed, validated input parameters for scientific and engineering software.*

[![Build](https://github.com/vrtulka23/scinumtools3/actions/workflows/c-cpp-build.yml/badge.svg)](https://github.com/vrtulka23/scinumtools3/actions/workflows/c-cpp-build.yml)
[![codecov](https://codecov.io/github/vrtulka23/scinumtools3/graph/badge.svg?token=8A25K1T7XM)](https://codecov.io/github/vrtulka23/scinumtools3)
[![Documentation](https://img.shields.io/badge/docs-online-blue)](https://vrtulka23.github.io/scinumtools3/)
![GitHub release](https://img.shields.io/github/v/release/vrtulka23/scinumtools3?include_prereleases)
[![PyPI version](https://badge.fury.io/py/scinumtools3.svg)](https://badge.fury.io/py/scinumtools3)   
[![Conda Version](https://anaconda.org/conda-forge/scinumtools3/badges/version.svg)](https://anaconda.org/conda-forge/scinumtools3)
[![vcpkg](https://img.shields.io/badge/vcpkg-available-blue)](https://vcpkg.io/en/package/scinumtools3.html)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg?logo=c%2B%2B&logoColor=white)
![Compilers](https://img.shields.io/badge/Compilers-GCC%2011%2B%20%7C%20Clang%2015%2B%20%7C%20MSVC%202022-blue)

---

## Overview

Every scientific or engineering application relies on input parameters. 
In many projects these parameters are scattered across configuration files, command-line options and source code, with little or no validation. 
As applications grow, this often leads to

- incorrect or inconsistent units
- invalid parameter values
- duplicated validation logic
- difficult-to-maintain configuration code
- discrepancies between simulation, analysis and post-processing tools

**SciNumTools3 (SNT)** solves this problem by providing a single, consistent parameter system that is

- **Strongly typed** – parameters have well-defined data types.
- **Unit-aware** – physical units are checked and converted automatically.
- **Validated** – ranges, constraints and relationships are verified before computation begins.
- **Expression-based** – parameters can depend on other parameters without recompilation.
- **Shared across C++, Python and the command line** – the same parameter definitions are used throughout an entire workflow.

Instead of implementing parameter parsing, unit conversions and validation separately in every application, SNT provides a **single source of truth** for scientific input data.

The result is fewer hidden assumptions, earlier error detection and more reliable scientific software.

## Getting Started

### Example of Use

Below is a quick example how to use the core functionality of `scinumtools3`.
For more examples and patterns please look into the ``tests``, ``exec``, ``apps`` and ``bindings`` folders.

#### Parameter definition 

``parameters.dip``

``` python
simulation
  title str = "Cylinder flow"        # strings
  mesh
    file str = "cylinder.msh"
      !format "[A-Za-z0-9_]+.msh"  # enforce string formats
  fluid
    density float = 998.2 kg/m3      # numbers with units
    viscosity float = 1.003e-3 Pa*s
  time
    dt float = 1e-3 s
    end float = 10 s
    steps int = ({?simulation.time.end} / {?simulation.time.dt})  # computed value
      !condition ({?} > 0)                    # validation
  solver
    restart_file str = none                   # optional value
  boundary[inlet]                             # node collections
    velocity float[3] = [1.0, 0.0, 0.0] m/s   # arrays
  boundary[outlet]
    pressure float = 0 Pa
  boundary[walls]
    type str = "no_slip"
      !options ["no_slip", "free_slip", "moving_wall"]   # allowed values
  output
    file str = "results.vtk"
    variables str[:] = ["velocity", "pressure", "vorticity"]
    every int = 100
# See the documentation for more features.
```

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

  puq::Quantity length("1*m");
  length = length.convert("km");
  std::cout << length.to_string() << std::endl;
  // 1e-3*km

  dip::DIP d;
  d.add_file("parameters.dip");
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

### Parameter Definition

`SciNumTools` is built around two domain-specific languages that form the foundation of the framework and enable consistent handling of scientific notation and dimensional analysis. The first language, [PUEL](docs/puel/specification.md), defines a formal notation system for representing physical units and quantities in a precise and machine-readable way. The second language, [DIPL](docs/dipl/specification.md), provides a textual specification format for defining dimensional parameters and their relationships. Together, these languages establish the core abstraction layer of `SciNumTools`, allowing scientific data, units, and dimensional constraints to be expressed in a structured, interoperable, and extensible manner.

#### Physical Units Expression Language - PUEL

Expressions of physical units must account for several important aspects, including the underlying unit system (such as SI, CGS, Atomic Units, or US Customary Units), unit prefixes and scaling factors (such as `kg`, `mm`, or `MJ`), the associated numerical values (whether scalar values or arrays), uncertainties arising from measurements, the relationship between base units and physical dimensions, as well as support for integer and fractional exponents of units.

`PUEL` provides a minimal, coherent and extensible notation that integrates all of these concerns into a unified representation. 

``` puel
# General form
<SYSTEM>_[<VALUE>]*<UNIT><EXPONENT>

# Examples
ESU_erg              # defining erg in ESU unit system
m2*kg*s-2            # definition of complex units
kg2*ms3:2*cm         # fractional exponents
1.346591(30)e27*kg   # uncertainties in measurements
[2, 3, 4, 5]*km      # arrays of values
```

Building upon this specification, the `PUQ` module of `SciNumTools` implements parsing, dimensional analysis, arithmetic operations, and unit conversion both within a single unit system and across different systems.

#### Dimensional Input Parameter Language - DIPL

The definition of input parameters for scientific and engineering software involves several recurring requirements that are common across many numerical codes. High-performance applications often require strongly typed parameters with explicitly defined numerical precision. Numerical values frequently carry associated physical units, and many parameters consist not only of scalar values, but also arrays, matrices, or tabulated datasets.

```dipl
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

In addition, parameters commonly require validation rules, numerical constraints, admissible ranges, or configurable options that govern their behavior and interpretation. Parameter definitions are often interdependent as well, where the validity, availability, or meaning of one setting depends on the values of others. `DIPL` provides a coherent and extensible framework for expressing these definitions, relationships, and constraints in a structured, machine-readable, and implementation-independent form.

---

## Installation

### Using package managers

SciNumTools can be installed using one of the following package managers.

#### PyPI (Python)

The Python bindings are available on PyPI and can be installed with `pip`:

```bash
pip install scinumtools3
```

Package: https://pypi.org/project/scinumtools3/

#### Conda

SciNumTools is available on the **conda-forge** channel and can be installed with Conda:

```bash
conda install conda-forge::scinumtools3
```

Package: https://anaconda.org/conda-forge/scinumtools3

#### vcpkg

```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

# Bootstrap (run once)
./bootstrap-vcpkg.sh      # macOS/Linux
# .\bootstrap-vcpkg.bat   # Windows

# Install SciNumTools
./vcpkg install scinumtools3
```

Package: https://vcpkg.io/en/package/scinumtools3

#### Homebrew (macOS)

```bash
brew tap vrtulka23/tap
brew install scinumtools3
```

Tap repository: https://github.com/vrtulka23/homebrew-tap

#### Conan

Clone the repository and create the package locally:

```bash
git clone https://github.com/vrtulka23/scinumtools3.git
cd scinumtools3

conan create .
```

The package can then be consumed from your local Conan cache in other CMake projects.

#### Docker

Preconfigured Docker images are provided for both Python users and SciNumTools developers.

Build the desired image from the repository root:

```bash
docker build -f packaging/docker/python/Dockerfile -t scinumtools3-python .
```

or

```bash
docker build -f packaging/docker/dev/Dockerfile -t scinumtools3-dev .
```

See [`packaging/docker/README.md`](packaging/docker/README.md) for detailed instructions on building, running, and using the available Docker images.

### From the source code

#### Download and install

1) Manually

   ```bash
   # download repository
   git clone https://github.com/vrtulka23/scinumtools3.git
   cd scinumtools3
   
   # compile
   cmake -G Ninja -B build
   cmake --build build
   
   # run tests
   ctest --test-dir build
   
   # install
   cmake --install build
   ```

2) Using setup script

   ```bash
   sudo ./setup.sh -b -t -i  # build, run tests, install
   ```

#### Link `SciNumTools3` in your `CMAKE` project

1) Find the package

   ```bash
   # find the `SNT` package
   find_package(snt REQUIRED)

   # link to your executable
   add_executable(${EXEC_NAME} ${SOURCE_FILES})
   target_link_libraries(${EXEC_NAME} PRIVATE snt-exs snt-puq snt-dip)
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

