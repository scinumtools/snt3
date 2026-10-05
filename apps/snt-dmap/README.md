# Dimensional Map Generator for PUQ module

The `snt dmap` command is used to precompute the dimensionality of derived units and quantities defined in the `SciNumTools v3` module `Physical Units and Quantities (PUQ)`.

All derived unit definitions for the supported unit systems are implemented as source definitions in the following directory:

``` bash
src/snt/puq/systems/system_*.cpp
```

Since all units are defined as [PUEL](https://github.com/scinumtools/snt3/blob/main/docs/puel/specification.md) expressions, performing dimensional analysis at runtime would introduce unnecessary overhead.
To avoid this, the unit definitions are precompiled into their base dimensional representations.

This tool reads the unit definitions, computes their dimensionality, and generates helper header files used during compilation.

The generated files are located in this directory.

``` bash
src/snt/puq/systems/dmaps/dmap_*.h
```

`snt dmap` is a developer tool. It overwrites these source headers rather than
producing a report. Run it from the project root only when updating PUQ unit
definitions, and review the generated diff before committing. The `-e` option
replaces the headers with empty placeholders; it is intended for build and test
workflows, not normal regeneration.

## Example of use

Build `snt` with `ENABLE_SNT_DMAP=ON` and run the command from the project root directory:

``` bash
./build/bin/snt dmap
```

This will regenerate and update the header files containing the precomputed dimensional representations of the newly added derived units defined in the supported unit systems.
