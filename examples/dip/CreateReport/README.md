# Mock Heat Flow settings and reports

This example shows two related outputs from one DIPL project: an evaluated
[run report](report.pdf) and a directory-based
[settings handbook](settings-handbook.pdf). The source tree
has short introductions for [settings](settings/README.md),
[schemas](schemas/README.md), and the [instrument catalog](catalog/README.md).

## Find what you need

- [Change a run value](settings/parameters.dip): duration and flow speed are
  marked as guided settings, with descriptions and basic visibility.
- [Understand ambient inputs](settings/environment.dip): conditions and the
  final sample-count adjustment are kept together.
- [Browse instrument records](catalog/instruments.dip): named entries have
  descriptions and share a reusable schema.
- [Inspect reusable declarations](schemas/README.md): defaults and types are
  separate from assigned values.
- [Read the evaluated run report](report.pdf): this applies the example
  override and shows its source.

## Generate the settings guide

From the repository root, generate the HTML reference and concise PDF handbook:

```sh
cmake --build build --target snt
build/bin/snt report docs --root examples/dip/CreateReport \
  --output-dir build/create-report-docs --format all \
  --cheatsheet-pdf docs/dipl/cheatsheet/dipl-cheatsheet.pdf
```

Open `build/create-report-docs/index.html` for the complete reference or
`build/create-report-docs/settings-reference.pdf` for the handbook. The
directory READMEs supply the navigation; source-level `?title` and `?descr`
introduce each file; `!setup` and `?visibility "basic"` identify notable fields
in larger files. The catalog is summarized by its named entries. Search also
shows `?category` and `?visibility` when supplied.

The guide documents the project defaults and leaves the local override file
empty during generation. The evaluated run report below uses that override.
For a field-by-field PDF, add `--pdf-detail full`. Omit `--cheatsheet-pdf` if
you do not want the DIPL quick reference appended.

## Generate the evaluated run report

This example demonstrates `snt report` with a DIPfile project. It includes two
custom units, three reusable schemas with mock publication metadata, multiple
source files, an overridden value, an applied modification, a readings table,
and a LaTeX introduction. The input files
are resolved relative to `DIPfile`.

The PDF contents page links to an **Override example** in the introduction.
The Parameter guide links every effective value to its full reference entry.
In the Parameters section, `experiment.flow_speed` has the effective value `3 m/s`
and shows its `2 m/s` declaration and `overrides.dip:1` replacement source.
`experiment.flow_distance` evaluates to `129.6 km` and names the speed and
duration values read during its calculation. The speed and duration entries
also show that they are used by `experiment.flow_distance`.
Also in the report,
`experiment.sample_count` is modified in `settings/environment.dip`, the readings table
shows its row count and column types, `experiment.sensor` and
`experiment.analysis` come from schemas, and
the instrument catalog is also registered, and `lab_length` and
`sample_period` appear under custom units. The PDF includes a
cover, linked contents page, and matching blocks for parameters, tables,
schemas, and custom units. The Schemas section lists the evaluated parameters
supplied by each schema.

From the repository root, generate a TeX report:

```sh
cmake --build build --target snt
build/bin/snt report --project examples/dip/CreateReport/DIPfile \
  --intro examples/dip/CreateReport/introduction.tex \
  --output build/create-report.tex
```

For a PDF, install a TeX compiler such as `pdflatex`, then run:

```sh
build/bin/snt report --project examples/dip/CreateReport/DIPfile \
  --intro examples/dip/CreateReport/introduction.tex \
  --title "Mock Heat Flow Study" --author "Example Research Team" \
  --date "2026-09-28" --report-version "1.0 demo" \
  --cheatsheet-pdf docs/dipl/cheatsheet/dipl-cheatsheet.pdf \
  --format pdf --output examples/dip/CreateReport/report.pdf
```

Brief++ can also render this project without a TeX compiler:

```sh
build/bin/snt report --project examples/dip/CreateReport/DIPfile \
  --format html --output build/create-report.html
build/bin/snt report --project examples/dip/CreateReport/DIPfile \
  --format md --output build/create-report.md
```

Other formats are `rst`, `typ`, `txt`, and `json`. The introduction file is
LaTeX and applies only to TeX and PDF reports.

Use `--tex-compiler lualatex` if your own report text contains Unicode that
`pdflatex` cannot typeset. TeX output itself needs no compiler. See
`build/bin/snt report --help` for other input options, including `--load` for a
saved DIPH5 environment.
