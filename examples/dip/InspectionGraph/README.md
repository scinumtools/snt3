# Inspection and dependency graph

This example opens a DIPfile with graph recording enabled. It prints evaluated
value metadata and source locations, an override history, table columns, an
array slice, forward and reverse dependencies, an EXS expression tree, a
condition, and the branch decision that selected a value. Unit expressions
remain value metadata; the graph does not expand their internal composition.

Build the normal example targets, then run from the repository root:

```sh
cmake -S . -B build -DENABLE_EXEC_EXAMPLES=ON
cmake --build build --target ExampleDipInspectionGraph
build/bin/ExampleDipInspectionGraph
```

The DIPL source declares a distance of `12 m`; `overrides.dip` changes it to
`15 m`. The resulting speed is `5 m/s`. The graph records that speed reads
`?experiment.distance` and `?experiment.duration`, while the provenance
history shows where the distance was declared and overridden.

Graph recording is optional. `open_artifact(project, true)` enables it for
this run; `open_artifact(project)` would evaluate the same values without
retaining graph events. The `?` in graph IDs identifies nodes in the current
environment. Imported nodes would use `source?path` IDs.
