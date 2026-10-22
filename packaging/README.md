# Packaging

`installers/` contains the no-admin CLI installers for Linux/macOS and Windows.
They use the precompiled GitHub release archives and also accept a local archive
for offline cluster installations. See the installation guide for user commands.

`docker/` contains development, Python, REST, and MCP container images.
The root `conanfile.py` packages the C++ libraries and `snt` executable.
The `with_server`, `with_viewer`, and `with_reports` options select optional
features. Consumers can declare the package as a `tool_requires` dependency
and use `VirtualBuildEnv` to put `snt` on `PATH`.

Publishing a GitHub release runs the Release SNT CLI workflow. It attaches
precompiled archives for Linux x86-64, macOS ARM64, and Windows x86-64 to the
published release. The installers consume those archives. The `snt hub`
command additionally needs Git and Python 3 for project adapters.

Run the local installer checks with:

```sh
python3 -m unittest discover -s packaging/tests
```

The vcpkg, Homebrew, and conda-forge recipes are maintained outside this
repository.
