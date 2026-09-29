# Create a DIP report

Open the [generated PDF](report.pdf) to see the result immediately.

This example demonstrates `snt report` with a DIPfile project. It includes two
custom units, two reusable schemas with mock publication metadata, multiple
source files, an overridden value, an applied modification, a readings table,
and a LaTeX introduction. The input files
are resolved relative to `DIPfile`.

The PDF contents page links to an **Override example** in the introduction.
In the Parameters section, `experiment.flow_speed` has the effective value `3 m/s`
and shows its `2 m/s` declaration and `overrides.dip:1` replacement source.
Also in the report,
`experiment.sample_count` is modified in `environment.dip`, the readings table
shows its row count and column types, `experiment.sensor` and
`experiment.analysis` come from schemas, and
`lab_length` and `sample_period` appear under custom units. The PDF includes a
cover, linked contents page, and matching blocks for parameters, tables,
schemas, and custom units.

From the repository root, build `snt` and generate a TeX report:

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
