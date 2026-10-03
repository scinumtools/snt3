# Parameter Viewer example

This is a synthetic DIP project for exercising the read-only SNT3 parameter
viewer. It is intentionally varied while remaining small enough to browse.

From the repository root, build the viewer and open the project:

```sh
cmake -S . -B build -DENABLE_SNT_VIEW=ON
cmake --build build --target parameter-viewer-snapshot
build/bin/snt view examples/dip/ParameterViewer/DIPfile
```

The checked-in `parameters.diph5` is the evaluated snapshot of this
`DIPfile`. It records the dependency graph and keeps source paths relative to
the snapshot directory. Open it to compare live-project and snapshot browsing:

```sh
build/bin/snt view examples/dip/ParameterViewer/parameters.diph5
```

When example builds are enabled, this target and a normal CMake build update
the snapshot after any project input changes. Commit the updated snapshot with
changes to its inputs.

For a text check without a graphical session:

```sh
build/bin/snt dip parse --project examples/dip/ParameterViewer/DIPfile --print
```

The snapshot generation command is equivalent to:

```sh
build/bin/snt dip parse --project examples/dip/ParameterViewer/DIPfile \
  --record-graph --relative-source-paths \
  --save examples/dip/ParameterViewer/parameters.diph5
```

| File | Purpose |
| --- | --- |
| `DIPfile` | Registers the custom unit, two DIPL sources, a raw array source, a raw table source, schema, override, and two code files. |
| `parameters.diph5` | Evaluated snapshot generated from `DIPfile`. |
| `parameters.dip` | Groups, scalar types, a value with a child, units, expressions, metadata, validation, map and list collections, and schema use. |
| `observations.dip` | Arrays, an array slice, inline and referenced tables, a formatted string, a compatible modification, a Boolean decision, and a conditional branch. |
| `reference.dip` | Values read through the named `reference` source. |
| `catalog.dip` | A second named source containing a map of devices. |
| `samples.txt` | Raw array data injected through the `raw_samples` source. |
| `table_samples.dipt` | Raw table data injected through the `table_samples` source. |
| `probe.dip` | Reusable probe schema. |
| `overrides.dip` | Replaces the declared channel length for this run. |

## Suggested tour

1. Open **DIPfile** to inspect the ordered registrations under Units, Sources,
   Schemas, Overrides, and Code. Select an entry to see its declared and
   resolved paths, then open the corresponding DIPfile line.
2. Expand `experiment.repeats`. It has a value (`4`) **and** a child
   (`warmup`), so its icon differs from an ordinary group or value.
3. Compare `experiment.geometry` (group), `experiment.materials` (map),
   `experiment.probes` (list), and `experiment.duration` (value).
4. Select `experiment.geometry.length`. The effective value is `3 m`; its
   declaration is in `parameters.dip`, and its override is in `overrides.dip`.
   The same target appears under **Overridden nodes**; selecting it shows the effective
   value and a link back to **Resolved nodes**. File locations in the
   inspector are shown relative to this project's `DIPfile`. Use **Open effective
   override** or **Open declaration** to view the corresponding source line in
   the read-only **Source** tab.
5. Select `experiment.average_speed`. It evaluates to `0.375 m/s` and depends
   on the effective length and duration. The inspector links to those inputs.
6. Open **Schemas → probe** to see where the schema was registered and follow
   its contributed values in **Resolved nodes**. Select
   `experiment.instrument.model` and the two probe items; the second probe
   changes its accuracy. Open **Units → sample_tick** to inspect its definition
   and jump to its declaration in `DIPfile`.
7. Select `experiment.lab_name`, then click `reference?lab_name` under
   **Depends on**. The browser opens **Named sources → reference → lab_name**, where
   the source's own value and declaration are shown. Then select
   `experiment.instrument_family` and follow its dependency into the separate
   **catalog** source and its `devices` map. Select
   `experiment.sample_interval` to see a value using that custom unit.
8. Select `experiment.temperatures` and `experiment.readings.temperature` to
   inspect an array and a table column. The current viewer summarizes arrays
   rather than displaying every element.
   Select **Raw named sources → raw_samples** and use **Open source** to view the raw
   text without DIPL coloring. `experiment.sample_values` reads that source.
   **Local sources → parameters.dip** and **Local sources → observations.dip** open the
   project code files with DIPL highlighting.
   **Block value sources → experiment.readings (table)** shows the inline table with
   its DIPL header highlighted and data rows in plain text. **Block value sources →
   experiment.sample_grid (array)** shows an array parsed from a string literal.
   **Raw named sources → table_samples** opens `table_samples.dipt` with the same presentation;
   `experiment.reference_readings` reads it.
9. Select `experiment.summary` to see string interpolation and
   `experiment.target_temperature` to see a compatible modification from
   `294 K` to `295 K`.
10. Select `experiment.is_fast` and `state` to see a logical expression and a
   conditional result. `state` evaluates to `"fast"` with the supplied override.

Edit a value in a `.dip` file, then use **File → Reload** or **Ctrl+R**. The
viewer keeps the last valid state if the edited project does not parse.
Source-node browsing currently requires a live DIPL or DIPfile parse; a
`.diph5` snapshot retains source identity but not source node values.
Source viewing is also disabled for `.diph5` snapshots because they do not
contain the complete source text. The Source tab highlights DIPL files and
table headers, and shows table data rows and other raw sources as plain text.
It offers a **Copy source** button; it does not edit files.
