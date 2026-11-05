# scinumtools3-sys

Raw declarations for the SNT3 C ABI. Use `scinumtools3` for the safe Rust API.

Install SNT3 with its C binding enabled, then set `SNT3_PREFIX` to its install
prefix before building. For a source checkout, `SNT3_BUILD_DIR` can point to a
CMake build directory containing `lib/libsnt-*.a` and
`bindings/c/libsnt-c.a`. HDF5 is discovered with `pkg-config`, or through
`SNT3_HDF5_LIB_DIR` and `SNT3_HDF5_LIB_NAME`.

Use a native SNT3 build that includes the bulk array C ABI functions in
`snt/c/dip.h`; earlier 0.10.0 binaries lack these symbols.
