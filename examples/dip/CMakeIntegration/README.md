# DIPL-driven CMake configuration

This standalone consumer uses an installed SNT CLI to evaluate `build.dip`
during CMake configuration. Set `build.tools` to `false` to omit the executable,
or change `build.standard` to select its C++ standard.

```sh
cmake -S examples/dip/CMakeIntegration -B build-cmake-dip-cmake-integration -DCMAKE_PREFIX_PATH=/path/to/snt/install
cmake --build build-cmake-dip-cmake-integration
```

Editing `build.dip` causes the next build to rerun CMake and update the targets.
See the [CMake integration reference](../../../bindings/cmake/README.md) for type checks,
external file dependencies, and host-tool selection when cross-compiling.
