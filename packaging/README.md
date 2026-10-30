# Packaging

`installers/` contains the no-admin CLI installers for Linux/macOS and Windows.
They select the local platform from either precompiled GitHub release bundle and also accept a local bundle
for offline cluster installations. See the installation guide for user commands.

`docker/` contains development, Python, REST, and MCP container images.
The root `conanfile.py` packages the C++ libraries and `snt` executable.
The `with_server`, `with_viewer`, and `with_reports` options select optional
features. Consumers can declare the package as a `tool_requires` dependency
and use `VirtualBuildEnv` to put `snt` on `PATH`.

Publishing a GitHub release runs the Release SNT CLI workflow. It attaches
`snt-vX.Y.Z-bundle.tar.gz` and `snt-vX.Y.Z-bundle.zip`; both contain builds
for Linux x86-64, macOS ARM64, and Windows x86-64. The installers extract only the matching platform build and
also accept the separate platform archives from older releases. The `snt hub`
command additionally needs Git and Python 3 for project adapters.

Run the local installer checks with:

```sh
python3 -m unittest discover -s packaging/tests
```

The vcpkg, Homebrew, and conda-forge recipes are maintained outside this
repository.
