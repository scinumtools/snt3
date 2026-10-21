# DIPL developer tools

This directory demonstrates `snt format` and `snt validate` with no extra
dependencies. `DIPfile` defines both tools' settings and includes only
`parameters.dip` in the project. The two invalid files are separate inputs so
the project itself remains valid.

From the repository root, after building `snt` (or replace `build/bin/snt`
with an installed `snt`), run:

```sh
build/bin/snt validate examples/dip/DevTools/DIPfile
build/bin/snt format --check examples/dip/DevTools/parameters.dip
build/bin/snt format examples/dip/DevTools/parameters.dip
```

The project validates successfully. The format check exits with status 1
because `parameters.dip` deliberately lacks normal spacing; the next command
prints the formatted result without changing the file. Use `format --write`
on a copy if you want to try rewriting it:

```sh
mkdir -p build/devtools-example
cp examples/dip/DevTools/DIPfile examples/dip/DevTools/parameters.dip build/devtools-example/
build/bin/snt format --write build/devtools-example/parameters.dip
build/bin/snt format --check build/devtools-example/parameters.dip
```

The final check exits with status 0.

To see validation diagnostics, run:

```sh
build/bin/snt validate examples/dip/DevTools/invalid-syntax.dip
build/bin/snt validate --json examples/dip/DevTools/invalid-syntax.dip
build/bin/snt validate examples/dip/DevTools/invalid-semantic.dip
```

Each invalid command exits with status 1. `invalid-syntax.dip` has three
separate syntax mistakes, but `validate.max_syntax_errors = 2` in the nearest
`DIPfile` limits the report to the first two. Change it to 3 and rerun to see
all three. `invalid-semantic.dip` parses successfully, then fails because
`"manual"` is not one of the declared `!options`.
