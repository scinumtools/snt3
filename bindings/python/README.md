# SciNumTools3 for Python

[![PyPI version](https://badge.fury.io/py/scinumtools3.svg)](https://pypi.org/project/scinumtools3/)
![Python](https://img.shields.io/badge/python-3.9%2B-blue.svg)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/vrtulka23/scinumtools3/blob/main/LICENSE)

SciNumTools3 provides unit-aware quantities and validated scientific configuration
for Python. Use it to convert physical units, perform dimension-aware calculations,
and load typed parameters whose units and constraints are checked at runtime.

The package includes:

- `scinumtools3.puq` for quantities, uncertainties, unit systems, and conversions
- `scinumtools3.dip` for typed, validated configuration files
- `scinumtools3.api` for formatted, command-style evaluation
- NumPy interoperability for array values

## Installation

SciNumTools3 requires Python 3.9 or later and depends on NumPy.

```bash
pip install scinumtools3
```

## Quick start

```python
from scinumtools3.puq import Quantity

distance = Quantity(1.5, "km")
time = Quantity("2*min")

speed = (distance / time).convert("m*s-1")
print(speed)  # 12.5*m*s-1
```

Incompatible dimensions are rejected instead of being combined silently.

## Quantities and units

`Quantity` accepts strings, Python numbers and sequences, and NumPy arrays. It
supports unit-aware arithmetic, comparisons, uncertainties, and conversion between
compatible units and unit systems.

PUEL (Physical Units Expression Language) expressions may contain scalar or array
values, uncertainties, dimensions, constants, and units. For example:

```python
from scinumtools3.puq import Quantity

lengths = Quantity("[2,3,4]*km")
measurement = Quantity("1.346591(30)e27*kg")
```

For conversions between unit systems, pass the target system and, when the
dimensions are ambiguous between systems, the physical-quantity context:

```python
from scinumtools3.puq import Quantity
from scinumtools3.puq.systems import ESU

current = Quantity("23*A").convert("statA", ESU, "I")
```

## Input parameters

`DIP` parses DIPL (Dimensional Input Parameter Language) definitions from strings or
files. DIPL provides typed parameters, units, constraints, dependencies, groups,
lists, maps, and references.

Given `parameters.dip`:

```text
simulation
  timestep float = 0.5 fs
    !condition ({.} > 0 fs)
  steps int = 1000000
    !condition ({.} > 1)
  duration float = ( {?simulation.timestep} * {?simulation.steps} ) fs
```

Parse it and access evaluated values through the environment cursor:

```python
from pathlib import Path
from scinumtools3.dip import DIP

dip = DIP()
dip.add_file(Path("parameters.dip"))
env = dip.parse()

duration = env["simulation.duration"]
print(duration.value, duration.units)
```

You can also add definitions directly:

```python
dip = DIP()
dip.add_string("length float = 2.5 dm")
dip.add_string("count int = 42")
env = dip.parse()
```

Cursor values are ordinary Python values: `bool`, `int`, `float`, `str`, or nested
lists. Use `cursor.to_numpy()` to obtain a NumPy array. A cursor also exposes its
`units`, `shape`, and child traversal methods. To convert a unit-bearing DIPL result,
construct a quantity from its value and the string representation of its units:

```python
from scinumtools3.puq import Quantity

length = env["length"]
length_cm = Quantity(length.value, length.units.to_string()).convert("cm")
print(length_cm)  # 25*cm
```

## Command-style API helpers

The `scinumtools3.api.puq` and `scinumtools3.api.dip` modules expose the same
application-oriented operations as the `snt` command-line interface:

```python
from scinumtools3.api.puq import PUQEval

command = PUQEval("1*cm + 2*dm")
print(command.execute())  # 21*cm
```

Use `Quantity` and `DIP` when you need typed Python objects; use the API helpers when
you want formatted command-style output without starting a subprocess.

## Documentation

- [Python usage guide](https://vrtulka23.github.io/scinumtools3/integrations/python.html)
- [Python API reference](https://vrtulka23.github.io/scinumtools3/api/python_api.html)
- [PUEL specification](https://vrtulka23.github.io/scinumtools3/puel/index.html)
- [DIPL specification](https://vrtulka23.github.io/scinumtools3/dipl/index.html)
- [GitHub repository](https://github.com/vrtulka23/scinumtools3)
- [Issue tracker](https://github.com/vrtulka23/scinumtools3/issues)

## License

SciNumTools3 is distributed under the [MIT License](https://github.com/vrtulka23/scinumtools3/blob/main/LICENSE).
