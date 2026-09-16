# Scientific Numerical Tools v3 (SciNumTools3, SNT)

Unit-safe, strongly typed, validated input parameters for scientific and
engineering software.

[![Build](https://github.com/vrtulka23/scinumtools3/actions/workflows/c-cpp-build.yml/badge.svg)](https://github.com/vrtulka23/scinumtools3/actions/workflows/c-cpp-build.yml)
[![codecov](https://codecov.io/github/vrtulka23/scinumtools3/graph/badge.svg?token=8A25K1T7XM)](https://codecov.io/github/vrtulka23/scinumtools3)
[![Documentation](https://img.shields.io/badge/docs-online-blue)](https://vrtulka23.github.io/scinumtools3/)
[![GitHub release](https://img.shields.io/github/v/release/vrtulka23/scinumtools3?include_prereleases)](https://github.com/vrtulka23/scinumtools3/releases)
[![PyPI version](https://badge.fury.io/py/scinumtools3.svg)](https://pypi.org/project/scinumtools3/)
[![Conda Version](https://anaconda.org/conda-forge/scinumtools3/badges/version.svg)](https://anaconda.org/conda-forge/scinumtools3)
[![vcpkg](https://img.shields.io/badge/vcpkg-available-blue)](https://vcpkg.io/en/package/scinumtools3.html)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg?logo=c%2B%2B&logoColor=white)
![Compilers](https://img.shields.io/badge/Compilers-GCC%2011%2B%20%7C%20Clang%2015%2B%20%7C%20MSVC%202022-blue)

SciNumTools provides a common representation for physical quantities and
validated scientific input parameters. Its C++17 libraries can be used from
C++, Python, the command line, CMake, and REST applications.

The project is built around two languages:

- **PUEL** describes values, units, uncertainties, arrays, and unit systems.
- **DIPL** describes typed input parameters, constraints, and relationships.

The [online documentation](https://vrtulka23.github.io/scinumtools3/) contains
the concepts, language specifications, tutorials, examples, and API reference.

## Quick example

Define parameters in a `parameters.dip` file:

```dipl
simulation
  title str = "Cylinder flow"
  fluid
    density float = 998.2 kg/m3
    viscosity float = 1.003e-3 Pa*s
  time
    timestep float = 1e-3 s
    end float = 10 s
    steps int = ({?simulation.time.end} / {?simulation.time.timestep})
  boundary[inlet]
    velocity float[3] = [1.0, 0.0, 0.0] m/s
```

The parsed environment remains available throughout the application, so the
same validated parameters can be passed between its components.

Use the PUQ and DIP APIs from C++:

```cpp
#include <snt/puq/quantity.h>
#include <snt/dip/dip.h>
#include <cstdint>
#include <iostream>

int main() {
  snt::puq::Quantity length("3.048*m");
  std::cout << length.convert("US_ft").to_string() << "\n";

  snt::dip::DIP dip;
  dip.add_file("parameters.dip");
  auto env = dip.parse();

  std::cout << env["simulation.fluid.density"].as<double>() << "\n";
  std::cout << env["simulation.time.steps"].as<int64_t>() << "\n";
}
```

Use the same functionality from Python:

```python
from scinumtools3.puq import Quantity
from scinumtools3.dip import DIP

length = Quantity(3.048, "m").convert("US_ft")
print(length.to_string())

dip = DIP()
dip.add_file("parameters.dip")
env = dip.parse()

print(env["simulation.fluid.density"].value)
print(env["simulation.time.steps"].value)
```

The command-line interface can consume the same definitions:

```console
$ snt puq convert "3.048*m" ft -s SI -S US
10*ft
$ snt dip parse -a file parameters.dip -r simulation.fluid.density --print
density = 998.2 kg*m-3
```

See the [quickstart](https://vrtulka23.github.io/scinumtools3/quickstart.html)
and [examples](https://vrtulka23.github.io/scinumtools3/examples/index.html)
for complete examples.

## Installation

### Python

```console
pip install scinumtools3
```

The package is also available from conda-forge:

```console
conda install conda-forge::scinumtools3
```

### C++

SciNumTools is available through vcpkg, Conan, and Homebrew (macOS):

```console
vcpkg install scinumtools3
brew tap vrtulka23/tap
brew install scinumtools3
```

For Conan, clone the repository and run `conan create .` from its root.

The Conan package builds the C++ libraries. Applications and language bindings
are configured separately.

### From source

```console
git clone https://github.com/vrtulka23/scinumtools3.git
cd scinumtools3
cmake -G Ninja -B build
cmake --build build
ctest --test-dir build
```

For build options, installation, Docker images, and package-manager details,
see the [installation guide](https://vrtulka23.github.io/scinumtools3/installation.html).

## CMake

After installation, find the package and link the modules required by an
application:

```cmake
find_package(snt REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE snt-api snt-dip)
```

The CMake integration can also parse a DIP file while configuring a project.
The resulting values are available as CMake variables for the rest of the
configuration process:

```cmake
find_package(snt REQUIRED)

snt_dip_get(
    FILE parameters.dip
    PATH simulation.time.steps
    OUT SIMULATION_STEPS
    REQUIRED
)
```

## Interfaces

- **C++ API:** modular libraries under `include/snt/`, documented with Doxygen
  and Breathe.
- **Python:** PUQ, VAL, DIP, and API bindings. VAL values are represented by
  native Python values and NumPy arrays.
- **CLI:** commands for PUQ conversion and DIP parsing.
- **C binding:** experimental and incomplete; use it only where its current
  scope is sufficient.
- **EXS:** available as a C++ module, without a standalone Python binding.
- **MAT:** currently available through the C++ API only.

The [integration guides](https://vrtulka23.github.io/scinumtools3/integrations/index.html)
cover Python, C, CLI, CMake, and REST usage.

## Development

The repository contains the C++ sources, Python bindings, command-line
applications, tests, examples, packaging recipes, and documentation.

SciNumTools v3 is the compiled successor to the original
[SciNumTools v2](https://github.com/vrtulka23/scinumtools), with the same focus
on units and validated scientific parameters.

Contributions and issue reports are welcome through
[GitHub](https://github.com/vrtulka23/scinumtools3).

## License

SciNumTools is released under the [MIT License](LICENSE).
