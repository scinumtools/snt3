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

Parse it into an environment:

```python
from pathlib import Path
from scinumtools3.dip import DIP

dip = DIP()
dip.add_file(Path("parameters.dip"))
env = dip.parse()

```

You can also add definitions directly:

```python
dip = DIP()
dip.add_string("length float = 2.5 dm")
dip.add_string("count int = 42")
env = dip.parse()
```

## Accessing nodes

There are two main ways to access nodes in an environment:

- **Cursor:** use `env["path"]` to inspect a known path or traverse groups and collections.
- **Select:** use `env.select(...)` to discover value nodes by path and tags and inspect snapshots of the results.

### Cursor: inspect a known path

```python
cursor = env["length"]
print(cursor.value, cursor.units, cursor.metadata.description)
```

A cursor accesses a path in the environment. Use `cursor.elements()` to traverse
list elements and `cursor.items()` to traverse named items in a map.
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

### Select: discover and inspect nodes

Use `select()` to discover values and inspect their tags and metadata directly:

```python
dip = DIP()
dip.add_string('''physics
  speed float = 2 m/s
    !tags ["export", "runtime", "hydro"]
    ?descr "Flow speed"
''')
env = dip.parse()
for node in env.select(
    "?physics.",
    tags_all=["export", "runtime"],
    tags_any=["hydro", "gravity"],
    tags_none=["internal", "deprecated"],
):
    print(node.name, node.value, node.tags, node.metadata.description)
```

The three filters combine with AND; omitted or empty filters impose no restriction.
Only explicitly assigned tags match: children do not inherit their parent's tags.
`?` selects the whole environment, `?physics.` selects a subtree including its
value-bearing root and collection members, and `?physics.speed` selects an exact
node. Results retain fully qualified paths in environment order, with each node
returned once. No matches returns an empty list.

Selected nodes are independent snapshots; selection does not modify the environment.
Tags are returned as a separate Python list, and metadata remains valid while its
Python wrapper is alive.

Additional request helpers remain available in the
[Python API reference](https://vrtulka23.github.io/scinumtools3/api/python_dip.html).

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
