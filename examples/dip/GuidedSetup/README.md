# Guided DIPL setup

The root `DIPfile` combines `build/DIPfile` and `runtime/DIPfile` through
`diplfile[]` entries. Build settings (`build.max_threads` and `build.trace`)
and runtime settings (`simulation.*`) therefore form one model. Each child
manifest resolves its files relative to its own directory.

Evaluate the combined project from the repository root:

```sh
build/bin/snt dip parse --project examples/dip/GuidedSetup/DIPfile \
  --request build.max_threads --print
build/bin/snt dip parse --project examples/dip/GuidedSetup/DIPfile \
  --request simulation.steps --print
```

A Rust application's `build.rs` can select just the compile settings from
this same project with `scinumtools3-build`:

```rust
use scinumtools3_build::{Config, RustType};

Config::new("../DIPfile")
    .rust_struct("crate::BuildSettings")
    .const_name("BUILD_SETTINGS")
    .field("max_threads", "build.max_threads", RustType::Usize)
    .field("trace", "build.trace", RustType::Bool)
    .generate("build_settings.rs")?;
```

The runnable [`rust-consumer`](rust-consumer) uses this `build.rs` mapping and
loads the root `DIPfile` at runtime to read `simulation.steps`. With a native
SNT3 C ABI build available, run from the repository root:

```sh
SNT3_BUILD_DIR="$PWD/build" cargo run \
  --manifest-path examples/dip/GuidedSetup/rust-consumer/Cargo.toml
```

Without `SNT3_BUILD_DIR`, Cargo builds the native SNT3 source. In either
case, CMake, a C++17 compiler, and HDF5 development files are required.

Try the interactive setup command from the repository root:

```sh
build/bin/snt dip setup examples/dip/GuidedSetup/DIPfile --output /tmp/snt-guided-setup.dip
```

The example marks user inputs with `!setup`. It demonstrates a boolean prompt,
human-readable `!options`/`!labels`, constrained integer and unit-bearing
numeric inputs, a formatted text input, a typed array expression, and a
required value without a default. The derived value is omitted from the prompts.
Each answer is checked immediately by the normal DIP parser. Invalid values
show the parser error and return to that prompt. The completed answers are
validated together again before writing a DIPL override file; the example
project is left unchanged.

Apply the generated override when evaluating the project:

```sh
build/bin/snt dip parse --project examples/dip/GuidedSetup/DIPfile \
  --input override_file /tmp/snt-guided-setup.dip --print
```
