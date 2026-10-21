# DIPL developer tools

This directory demonstrates `snt format`, `snt validate`, and `snt lint` with
no extra dependencies. `DIPfile` defines the tools' settings and includes only
`parameters.dip` in the project. The invalid and lint example files are
separate inputs, so the project itself remains valid.

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

To see advisory lint findings on valid DIPL, run:

```sh
build/bin/snt lint examples/dip/DevTools/lint-advisory.dip
build/bin/snt lint --json examples/dip/DevTools/lint-advisory.dip
build/bin/snt lint examples/dip/DevTools/DIPfile
```

The first two commands exit with status 1: `output` is explicitly public but
lacks a `?descr`, its numeric choices lack `!labels`, one choice is repeated,
and `old_output` is marked deprecated. The JSON output distinguishes a valid
input (`valid: true`) from a clean lint result
(`clean: false`). The project itself is clean, so the last command exits 0.
Try changing one of the four enabled `lint` settings to `false` in `DIPfile`
to disable that rule. The two optional rules are off by default. To try them,
set `warn_broken_metadata_references` and `warn_unused_schemas` to `true`,
then run:

```sh
build/bin/snt lint examples/dip/DevTools/lint-optional.dip
```

This reports the path-like `?requires` target that is absent from the evaluated
model and the schema `spare`, which has no evaluated application. Reusable
schema libraries are a reason to leave the unused-schema rule disabled.
