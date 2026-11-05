# scinumtools3-sys

Raw declarations for the SNT3 C ABI. Use `scinumtools3` for the safe Rust API.

Starting with the next release, the crate includes the native SNT3 source and
builds it automatically with CMake. Users do not need to install SNT3
separately. A C++17 compiler, CMake, and HDF5 development libraries are still
required. HDF5 is linked
through `pkg-config`, or through `SNT3_HDF5_LIB_DIR` and
`SNT3_HDF5_LIB_NAME`.

To reuse an existing native installation, set `SNT3_PREFIX` to its install
prefix. For a source checkout, `SNT3_BUILD_DIR` can point to a CMake
build directory containing `lib/libsnt-*.a` and
`bindings/c/libsnt-c.a`. These settings skip the bundled build.

Use a native SNT3 build that includes the bulk array C ABI functions in
`snt/c/dip.h`; earlier 0.10.0 binaries lack these symbols.
