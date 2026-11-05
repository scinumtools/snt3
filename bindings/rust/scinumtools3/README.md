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

The native SNT3 libraries must be installed with `ENABLE_BINDING_C=ON`.
Set `SNT3_PREFIX` to their installation prefix, or `SNT3_BUILD_DIR` to a
CMake build directory. HDF5 must also be installed. See the sys crate README
for the linking environment variables.
