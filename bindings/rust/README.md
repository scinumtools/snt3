# Rust binding

This Cargo workspace contains the raw `scinumtools3-sys` C ABI declarations
and the safe `scinumtools3` API. The Rust layer owns native handles and
translates errors; SNT3's C++ core parses and evaluates DIPL and reads/writes
DIPH5.

For a local source build:

```sh
cmake -S . -B build-rust -DENABLE_BINDING_C=ON \
  -DENABLE_BINDING_PYTHON=OFF -DENABLE_UNIT_TESTS=OFF \
  -DENABLE_EXEC_APPS=OFF -DENABLE_EXEC_EXAMPLES=OFF \
  -DENABLE_SNT_REPORT=OFF
cmake --build build-rust --target snt-c
SNT3_BUILD_DIR="$PWD/build-rust" cargo test --manifest-path bindings/rust/Cargo.toml
```

Starting with the next release, the crate bundles the SNT3 native source, so
Cargo builds it when neither `SNT3_PREFIX` nor `SNT3_BUILD_DIR` is set. CMake, a C++17
compiler, and HDF5 development libraries are still required. The sys crate
README documents HDF5 discovery settings.

When a GitHub release is published, the release workflow checks out its tag,
checks the tag against `CODE_VERSION` in `settings.env`, and publishes the
crates with that version. For a manual run, enter an existing published release
tag in Actions. The workflow builds the Rust binding and native libraries from
the current `main` branch, then assigns the entered tag's version to all three
crates. This permits publishing crates for an earlier release tag from newer
source. In either mode, it updates the Cargo versions and lockfile in its
checkout, runs the Rust tests, and publishes `scinumtools3-sys`,
`scinumtools3`, then `scinumtools3-build` in dependency order. Before
publishing, it snapshots the native source into the sys crate and verifies
that the packaged crate builds without an external SNT3 installation.
Configure the repository secret
`CRATES_IO_TOKEN` with a crates.io API token authorized to publish all three
crates. For an initial publication, use a token that can create the crates;
later tokens may be scoped to the existing crates.

The published crates use the selected release version even if the Cargo
manifests in the source still have the previous version. Run
`python3 bindings/rust/sync_version.py --write` only if you want local
Cargo metadata to reflect a new SNT3 version before release.

The native installation must include the bulk array C ABI functions added
with this binding; the original SNT3 0.10.0 libraries do not include them.
A crate published manually as 0.10.0 from newer `main` source needs the
newer native C ABI, despite its 0.10.0 crate version. The system SNT3 C ABI
remains experimental.
