# Using DIPL in CMake projects

SNT installs a CMake package named `snt`. Its `snt_dip_get` helper evaluates a DIPL
configuration with the installed CLI at configure time, so its values can control
`if()`, `add_subdirectory()`, `find_package()`, and target properties.

```cmake
find_package(snt CONFIG REQUIRED)

snt_dip_get(
    FILE config.dip
    PATH build.python
    TYPE BOOL
    OUTPUT ENABLE_PYTHON
)

if(ENABLE_PYTHON)
    find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
endif()
```

```dipl
build
    python bool = false
    standard int = 17
```

## Helper contract

```cmake
snt_dip_get(FILE <file> PATH <node> OUTPUT <variable>
            [TYPE BOOL|INTEGER|FLOAT|STRING]
            [DEPENDS <file>...])
```

- The request must select exactly one defined scalar without units. Arrays and
  unit-bearing quantities are rejected rather than flattened or stripped of units.
- `TYPE` checks the actual DIPL type, without coercion. Use `TYPE BOOL` for build
  switches: a string such as `"false"` is not a DIPL boolean.
- Boolean output is `true` or `false`, both understood by CMake's `if()`.
  Strings arrive without surrounding quotes, preserving their contents. Quote
  string variables when passing them to other CMake commands, especially if they
  contain semicolons.
- `OUTPUT` sets a normal variable in the caller's scope. It does not create or
  overwrite a cache entry. `OUT` is a supported alias. The old `REQUIRED` keyword
  remains accepted; every failed query now stops configuration, even without it.
- `PATH` accepts `build.python` or `?build.python`. Relative `FILE` and `DEPENDS`
  paths are resolved against `CMAKE_CURRENT_SOURCE_DIR`. The CLI runs in the
  configuration file's directory, allowing relative external-source paths.
- The main file and CLI executable trigger reconfiguration when changed. List
  additional DIPL source files in `DEPENDS`; transitive file dependencies are not
  discovered automatically. Values are evaluated again at each configuration.
- Invalid syntax, missing nodes, type mismatches, and CLI failures are fatal.
  A query has a 30-second timeout. Empty strings and `false` are valid results.

Use normal variables for switches controlled by DIPL. If a switch should instead
be user-overridable through `-D`, explicitly use the DIPL result as the default
of `option()`. CMake caches that default, so later DIPL edits will not replace a
user's cached choice.

## Executable discovery

`find_package(snt CONFIG REQUIRED)` exposes `SNT_EXECUTABLE` and, when the CLI
was installed, the imported executable target `snt::snt`. The helper uses the
executable from that installation, not an unrelated `snt` on `PATH`.

Set `-DSNT_EXECUTABLE=/absolute/path/to/snt` to choose another host executable.
When cross-compiling, this override is required: the target-platform executable
cannot generally run during configuration. A library-only SNT installation can
still be found without a CLI; only calling the helper requires one.

The CLI must already exist at configure time. Adding SNT through
`add_subdirectory()` does not build it early enough to query it during that same
configuration. Use an installed host SNT for this workflow.

## CLI equivalent

```sh
snt dip parse --input file config.dip --request build.python --value --type bool
```

`--value` writes one raw scalar followed by a newline. Diagnostics go to stderr
and failures return a nonzero status. `--print` retains the human-readable
`name = value` form. Input can also be supplied through `--input file config.dip`;
Input is supplied through `--input file config.dip`; `--add` / `-a` remain aliases for `--input` / `-i`.

A [standalone example](../examples/cmake-dipl) demonstrates creating or removing a
build target based on a DIPL boolean.
