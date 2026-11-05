# scinumtools3

Safe Rust access to SciNumTools3's DIPL and DIPH5 core through its C ABI.
The parser, validation, and HDF5 work run in C++; Rust owns the handles and
returns `Result` values.

```rust,no_run
use scinumtools3::Project;

let project = Project::load("DIPfile")?;
let steps = project.node("simulation.steps")?.as_i64()?;
println!("steps = {steps}");
# Ok::<(), scinumtools3::Error>(())
```

`Project::load` accepts a DIPfile, a project directory containing DIPfile,
a standalone `.dip` or `.dipl` source, or a `.diph5` archive. For manually
registered sources, use `Project::new`, `add_source`/`add_file`/`add_project`,
and `parse`. The C binding currently remains experimental, so the Rust crate
has the same 0.x compatibility expectations.

Starting with the next crate release, Cargo builds the bundled SNT3 native
libraries automatically. Install a C++17 compiler, CMake, and HDF5 development
libraries; a separate SNT3 installation is not needed. To reuse an existing
SNT3 build, set `SNT3_PREFIX` to its
installation prefix or `SNT3_BUILD_DIR` to its CMake build directory.
See the sys crate README for HDF5 linking settings.
