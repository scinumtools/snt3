Rust binding
============

The Rust workspace in ``bindings/rust`` wraps the SNT3 C ABI. Parsing,
validation, DIPH5 I/O, and value conversion remain in the C++ core. The
``scinumtools3-sys`` crate contains raw declarations; ``scinumtools3`` owns
the native handles and exposes ``Result``-returning methods.

Build SNT3 with ``ENABLE_BINDING_C=ON`` and install it. Set ``SNT3_PREFIX`` to
the installation prefix before using Cargo. In a source checkout, set
``SNT3_BUILD_DIR`` to the CMake build directory instead. HDF5 must be
available through ``pkg-config`` or the environment variables described in
``bindings/rust/scinumtools3-sys/README.md``.

.. code-block:: console

   cmake -S . -B build-rust -DENABLE_BINDING_C=ON \
       -DENABLE_BINDING_PYTHON=OFF -DENABLE_UNIT_TESTS=OFF \
       -DENABLE_EXEC_APPS=OFF -DENABLE_EXEC_EXAMPLES=OFF \
       -DENABLE_SNT_REPORT=OFF
   cmake --build build-rust --target snt-c
   SNT3_BUILD_DIR="$PWD/build-rust" cargo test --manifest-path bindings/rust/Cargo.toml

``Project::load`` accepts a ``DIPfile``, a directory containing one, a
standalone ``.dip`` or ``.dipl`` source, or a ``.diph5`` archive:

.. code-block:: rust

   use scinumtools3::Project;

   let project = Project::load("DIPfile")?;
   let steps = project.node("simulation.steps")?.as_i64()?;
   let samples = project.node("simulation.samples")?.as_i64_array()?;
   println!("{steps}: {samples:?}");

The binding also provides read-only hierarchy traversal, units, metadata,
provenance, overrides, DIPH5 saving, and comparison. Register sources and
overrides before calling ``parse``; create a new ``Project`` for a different
configuration. The underlying C ABI is currently experimental, so the Rust
binding retains 0.x compatibility expectations.
