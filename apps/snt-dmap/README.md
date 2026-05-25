# Dimensional Map Generator for PUQ module

This executable is used to precompute the dimensionality of derived units and quantities defined in the `SciNumTools v3` module `Physical Units and Quantities (PUQ)`.

All derived unit definitions for the supported unit systems are implemented as source definitions in the following directory:

``` bash
src/snt/puq/systems/system_*.cpp
```

Since all units are defined as [PUEL](https://github.com/vrtulka23/scinumtools3/blob/main/docs/puel/specification.md) expressions, performing dimensional analysis at runtime would introduce unnecessary overhead.
To avoid this, the unit definitions are precompiled into their base dimensional representations.

This tool reads the unit definitions, computes their dimensionality, and generates helper header files used during compilation.

The generated files are located in this directory.

``` bash
src/snt/puq/systems/dmaps/dmap_*.h
```

## Example of use

Build the main library and run the executable from the project root directory:

``` bash
./build/bin/snt-dmap
```

This will regenerate and update the header files containing the precomputed dimensional representations of the newly added derived units defined in the supported unit systems.
