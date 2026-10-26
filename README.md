# Scientific Numerical Tools v3 `{?SNT3}`

Unit-safe, strongly typed, validated input parameters for scientific and
engineering software.

Focus on your science. Leave the parameter plumbing to SciNumTools.

[![Build](https://github.com/scinumtools/snt3/actions/workflows/c-cpp-build.yml/badge.svg)](https://github.com/scinumtools/snt3/actions/workflows/c-cpp-build.yml)
[![Codecov](https://img.shields.io/codecov/c/github/scinumtools/snt3?color=7B61A8)](https://codecov.io/github/scinumtools/snt3)
[![GitHub release](https://img.shields.io/github/v/release/scinumtools/snt3?include_prereleases)](https://github.com/scinumtools/snt3/releases)
[![PyPI version](https://img.shields.io/pypi/v/scinumtools3.svg)](https://pypi.org/project/scinumtools3/)
[![Conda Version](https://img.shields.io/conda/vn/conda-forge/scinumtools3?label=conda&color=orange)](https://anaconda.org/conda-forge/scinumtools3)
[![vcpkg](https://img.shields.io/vcpkg/v/scinumtools3?label=vcpkg)](https://vcpkg.io/en/package/scinumtools3.html)
[![Homebrew version](https://img.shields.io/badge/dynamic/regex?url=https%3A%2F%2Fraw.githubusercontent.com%2Fscinumtools%2Fhomebrew-tap%2Fmain%2FFormula%2Fscinumtools3.rb&search=refs%2Ftags%2F%28v%5B%5E%2F%22%20%5D%2B%29%5C.tar%5C.gz&replace=%241&label=homebrew&color=orange)](https://github.com/scinumtools/homebrew-tap/blob/main/Formula/scinumtools3.rb)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00897B.svg?logo=c%2B%2B&logoColor=white)
![Compilers](https://img.shields.io/badge/Compilers-GCC%2011%2B%20%7C%20Clang%2015%2B%20%7C%20MSVC%202022-00897B)
[![Documentation](https://img.shields.io/badge/docs-online-blue)](https://scinumtools.github.io/snt3/)
[![SNT3: DIPL](https://img.shields.io/badge/SNT3-DIPL-2980b9)](https://scinumtools.github.io/snt3/initiative.html)

SciNumTools provides a common representation for physical quantities and
validated scientific input parameters. Its C++17 libraries can be used from
C/C++, Python, the command line, and CMake.

The project is built around two languages:

- **PUEL** describes values, units, uncertainties, arrays, and unit systems.
  See the [PUEL language specification](https://scinumtools.github.io/snt3/puel/index.html).
- **DIPL** describes typed input parameters, constraints, and relationships.
  See the [DIPL language specification](https://scinumtools.github.io/snt3/dipl/index.html).

DIPL can serve as the single source of validated truth for a scientific
model: values, units, constraints, derived relationships, and provenance stay
together in one portable definition rather than being distributed across
configuration files and application code.

The [online documentation](https://scinumtools.github.io/snt3/) contains
the concepts, language specifications, tutorials, examples, and API reference.
The official SNT3 benchmarking project is
[scinumtools/snt-bench](https://github.com/scinumtools/snt-bench).

![DIPL architecture: scientific parameters include physical units, hierarchies, references, constraints, dependencies, sources, and provenance; the C++ core shares them through C++, Python, C, WebAssembly and JavaScript, command-line, CMake, REST, MCP, and Parameter Viewer interfaces.](docs/img/snt-schema-readme-interfaces-provenance.png)

## Why DIPL?

Scientific software often begins with a small configuration file, then grows
into values spread across input files, unit conversions, validation code,
derived constants, and undocumented assumptions. That makes a model harder to
review, reproduce, and safely reuse in another tool or language.

DIPL was created to make the parameter definition itself the authoritative
model interface. It keeps physical units, types, constraints, hierarchical
structure, references, expressions, metadata, and provenance together, then
exposes one evaluated and validated environment to every part of an
application. The result is less duplicated glue code and a clearer boundary
between a scientific model's assumptions and its implementation.

## Features

- **Unit-aware quantities:** [Calculate and convert](https://scinumtools.github.io/snt3/modules/puq/quantities.html)
  values with units, uncertainties, arrays, prefixes, and unit systems.
- **Validated parameter models:** Define [typed DIPL hierarchies](https://scinumtools.github.io/snt3/modules/dip/basic-usage.html)
  with constraints, expressions, dependencies, and metadata.
- **Reviewable changes:** [Inspect provenance, preview overrides, and compare](https://scinumtools.github.io/snt3/modules/dip/inspection.html)
  evaluated environments before accepting new settings.
- **Portable results:** Save [DIPH5 snapshots](https://scinumtools.github.io/snt3/modules/dip/persistence.html)
  with source-content hashes, or [generate files](https://scinumtools.github.io/snt3/modules/dip/generation.html)
  for C++, C, Fortran, Rust, Julia, JSON, and YAML.
- **Readable reports:** [Generate reports](https://scinumtools.github.io/snt3/modules/dip/report.html)
  with effective values, units, overrides, provenance, schemas, and references
  in TeX, PDF, Markdown, HTML, and other formats.

## Interfaces

- **C++ API:** [C++17 reference](https://scinumtools.github.io/snt3/api/index.html)
  for quantities, DIPL, and application commands.
- **Python:** [Bindings](https://scinumtools.github.io/snt3/integrations/python.html)
  for PUQ, DIP, and API, including NumPy arrays.
- **C binding:** Experimental [PUQ and DIPL interfaces](https://scinumtools.github.io/snt3/integrations/c.html)
  using opaque handles.
- **WebAssembly and JavaScript:** [Browser-side tools](https://scinumtools.github.io/snt3/integrations/web.html)
  for DIPL inspection and PUQ conversion.
- **Command line:** [PUQ, DIPL, export, and report commands](https://scinumtools.github.io/snt3/integrations/cli.html),
  plus [formatting, validation, and linting](docs/source/integrations/devtools.rst).
- **Parameter Viewer:** [Graphical browser and source editor](https://scinumtools.github.io/snt3/integrations/viewer.html)
  for DIPL projects and DIPH5 snapshots, with dependency and provenance views;
  edit project sources with check, format, and find/replace tools.
- **CMake:** [Package integration and DIPL evaluation](https://scinumtools.github.io/snt3/integrations/cmake.html)
  during configuration.
- **REST API server:** [Local HTTP service](https://scinumtools.github.io/snt3/integrations/rest.html)
  for the command-oriented PUQ and DIP API.
- **MCP for AI agents:** Native [`snt mcp` server](docs/source/integrations/mcp.rst)
  with DIPL and PUQ tools over stdio or Streamable HTTP.
- **Docker:** [Images](packaging/docker/README.md)
  for Python, development, REST, and MCP.

See the [documentation index](https://scinumtools.github.io/snt3/)
for the complete usage guides.

## Quick example

Define parameters in a `parameters.dip` file:

```dipl
simulation
  title str = "Cylinder flow"
  fluid
    density float = 998.2 kg/m3
      !condition ({.} > 0 kg/m3)
    viscosity float = 1.003e-3 Pa*s
  time
    timestep float = 1e-3 s
    end float = 10 s
    steps int = ({?simulation.time.end} / {?simulation.time.timestep})
  boundary[inlet]
    velocity float[3] = [1.0, 0.0, 0.0] m/s
  solver
    type str = "steady"
      !options ["steady", "transient"]
```

The same definition can be read from C++, Python, and the command line. For
persistence, exports, and reports, see the [CLI guide](https://scinumtools.github.io/snt3/integrations/cli.html).

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
$ snt dip parse -i file parameters.dip -r simulation.fluid.density --print
density = 998.2 kg*m-3
```

See the [quickstart](https://scinumtools.github.io/snt3/quickstart.html)
and [examples](https://scinumtools.github.io/snt3/examples/index.html)
for complete examples.

## Installation

For Python, install from PyPI with `pip install scinumtools3` or from
conda-forge with `conda install conda-forge::scinumtools3`.

For C++, use `vcpkg install scinumtools3`. On macOS, use
`brew tap scinumtools/tap` followed by `brew install scinumtools3`.

To install the precompiled `snt` command without administrator rights on
Linux x86-64 or macOS ARM64, run:

```sh
curl -fsSLO https://raw.githubusercontent.com/scinumtools/snt3/main/packaging/installers/install-cli.sh
sh install-cli.sh
```

On Windows x86-64, use the [PowerShell installer](packaging/installers/install-cli.ps1).
See the [installation guide](https://scinumtools.github.io/snt3/installation.html)
for Windows commands, offline installation, Conan, and source builds.

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

## One parameter layer for many scientific codes

<a href="https://scinumtools.github.io/snt3/initiative.html"><img align="left" src="docs/img/snt-initiative-readme.png" alt="A parameter model is validated and translated into a scientific code's native input file" width="180" height="180" hspace="16" style="margin-right:30px"></a>

We are inviting maintainers of established, open-source scientific codes to
try a shared, optional parameter layer. A project-specific adapter can turn a
DIPL definition into its existing native input file, while SciNumTools handles
units, constraints, dependencies, and parameter documentation. The first step
keeps the solver and its native parser in place. [Read about the initiative
and see the full flyer](https://scinumtools.github.io/snt3/initiative.html).
The long-term aim is direct integration where a project judges the model and
tooling stable and trustworthy.
Explore related projects and integrations in the
[SNT Hub registry](https://scinumtools.github.io/snt-hub/#registry).

With SNT installed, create a workspace and choose a Hub project:

```bash
mkdir study && cd study
snt hub list
snt hub fetch PROJECT
snt hub examples
snt hub setup EXAMPLE
```

Replace `PROJECT` and `EXAMPLE` with names listed by `list` and `examples`.
Generated files are placed in `runs/EXAMPLE`;
see the [SNT Hub CLI guide](https://scinumtools.github.io/snt3/integrations/cli.html#pinned-code-examples-with-snt-hub) for prerequisites and overrides.

<br clear="left">

## AI-assisted scientific setup

AI assistants, including large language models (LLMs), can draft scientific
setups, but those drafts need explicit checks. DIPL gives them a readable,
structured description of run settings and references to initial-condition
files. SciNumTools checks declared types, units, constraints, and dependencies;
a code-specific adapter can check further requirements and write the code's
native input files. This makes an AI-assisted setup easier to inspect and test
before a run. Scientific review is still needed to judge whether the choices
are physically appropriate.

The current tools let an assistant or script [list and describe](https://scinumtools.github.io/snt3/integrations/cli.html#cli-semantic-interface)  evaluated
parameters, then preview proposed overrides as JSON.
The same operations are available through the [C++ inspection API](https://scinumtools.github.io/snt3/modules/dip/inspection.html#cpp-semantic-interface)
and [Python bindings](https://scinumtools.github.io/snt3/integrations/python.html#python-semantic-interface).

The native [`snt mcp` interface](docs/source/integrations/mcp.rst)
lets AI agents validate, compare, and export DIPL candidates.
Try the runnable [UsingMCP example](examples/dip/UsingMCP).

## Built with SciNumTools

[Nuclide Atlas](https://github.com/vrtulka23/nuclide-atlas) is a unit-aware
nuclear-isotope database and decay-chain calculator built with SciNumTools3.
It uses DIPL for validated nuclide data, scenarios, provenance, and CMake build
configuration.

See [more applications and examples](docs/source/examples/index.rst).

## Development

The repository contains the C++ sources, Python bindings, command-line
applications, tests, examples, packaging tools, and documentation.

SciNumTools v3 is the compiled successor to the original
[SciNumTools v2](https://github.com/scinumtools/snt2), with the same focus
on units and validated scientific parameters.

Contributions and issue reports are welcome through
[GitHub](https://github.com/scinumtools/snt3). See
[CONTRIBUTING.md](CONTRIBUTING.md) for the development workflow.

## License

SciNumTools is released under the [MIT License](LICENSE).
