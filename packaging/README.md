# Packaging

The Homebrew and vcpkg recipes build the libraries and unified `snt` executable,
including `snt server`. They declare HDF5 and cpp-httplib dependencies explicitly;
GitHub source archives do not contain Git submodule contents. The archive-based
recipes can fetch the pinned header-only Brief++ source separately for DIP reports.
The vcpkg port enables reports with the `reports` feature
(`scinumtools3[reports]`); its default build omits Brief++ and report generation.
Source builds use `ENABLE_SNT_REPORT`, which defaults to on when DIP is enabled.
Local source builds can initialize `external/briefpp` or set
`SNT_BRIEFPP_INCLUDE_DIR`. Server builds accept `SNT_HTTPLIB_INCLUDE_DIR` and
require cpp-httplib 0.46.0 or newer. Homebrew, vcpkg, and Conan include the
unified `snt` executable. The vcpkg and Conan packages enable `snt view`;
Homebrew disables it. Conan's developer-only `snt dmap` command is disabled.
Conda and PyPI package Python bindings without application commands. The vcpkg
recipe fetches pinned Dear ImGui and GLFW sources because GitHub archives omit
submodules. On Linux it builds GLFW's X11 backend and requires the system's
OpenGL and X11 development libraries.

Conan packages the C++ libraries and executable, and propagates the HDF5
C-library dependency to consumers. Create it from a checkout with the
`external/briefpp`, `external/cpp-httplib`, `external/glfw`, and `external/imgui`
submodules initialized. Building the viewer also requires the platform's OpenGL
and window-system development libraries. The `with_server` and `with_viewer`
Conan options default to `True`; disable either when its dependencies are not
available. Consumers that need `snt` on `PATH` can declare the package as a
`tool_requires` dependency and generate `VirtualBuildEnv`. Conda and PyPI
package the Python bindings with application features disabled. Conda obtains
HDF5 from its host environment and uses its runtime pinning.
The development and Python Docker images install HDF5 development files; the REST
image builds and starts `snt server`.

## Preparing a release

Archive-based recipes retain their last published version and checksums until a
new release archive is available. Do not change the version while retaining an
old checksum, or use a locally generated archive's checksum for a GitHub URL.

After publishing the intended Git tag, update all archive-based recipes together:

```sh
python3 packaging/update_release.py 0.9.0
python3 packaging/update_release.py 0.9.0 --check
```

The command downloads the GitHub tag archive, checks its `CODE_VERSION` and unified `snt server` / `snt dmap` support, and
computes SHA-256 and SHA-512 digests for Homebrew, vcpkg, and Conda. It updates the
vcpkg manifest version as well. It does not publish packages or modify the project
version. `--archive /path/to/downloaded.tar.gz` supports an already downloaded copy
of the same GitHub archive. Download and validation failures leave recipes unchanged.

Homebrew, vcpkg, and Conda are pinned to the published 0.9.0 archive with
verified checksums. Conan reads its version from `settings.env`; PyPI reads
its version from `pyproject.toml`.

## Local checks

```sh
python3 -m unittest discover -s packaging/tests
ruby -c packaging/homebrew/scinumtools3.rb
```

Before publishing, also build the recipes with their native package managers,
verify `snt --version` and `snt server --help` for CLI packages, and import
`scinumtools3` for Python packages. Docker builds require a running Docker daemon.
