# Create a DIP report

This example demonstrates `snt docs` with a small DIPfile project. It includes
a custom unit, a reusable schema with mock publication metadata, an overridden
value, and an optional LaTeX introduction. The input files are resolved relative
to `DIPfile`.

In the report, `experiment.flow_speed` has the effective value `3 m/s`,
`experiment.sensor` comes from the `sensor` schema, and `lab_length` appears
under custom units.

From the repository root, build `snt` and generate a TeX report:

```sh
cmake --build build --target snt
build/bin/snt docs --project examples/dip/CreateDocs/DIPfile \
  --intro examples/dip/CreateDocs/introduction.tex \
  --output build/create-docs-report.tex
```

For a PDF, install a TeX compiler such as `pdflatex`, then run:

```sh
build/bin/snt docs --project examples/dip/CreateDocs/DIPfile \
  --intro examples/dip/CreateDocs/introduction.tex \
  --format pdf --output build/create-docs-report.pdf
```

Use `--tex-compiler lualatex` if your own report text contains Unicode that
`pdflatex` cannot typeset. TeX output itself needs no compiler. See
`build/bin/snt docs --help` for other input options, including `--load` for a
saved DIPH5 environment.
