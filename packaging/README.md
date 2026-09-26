# Packaging

The Homebrew and vcpkg recipes build the libraries and unified `snt` executable,
including `snt server`. They declare HDF5 and cpp-httplib dependencies explicitly;
GitHub source archives do not contain Git submodule contents. Server builds accept
`SNT_HTTPLIB_INCLUDE_DIR` and require cpp-httplib 0.46.0 or newer. The viewer remains
disabled until implemented.

Conan packages the C++ libraries and propagates the HDF5 C-library dependency to
consumers. Conda and PyPI package the Python bindings with application features
disabled. Conda obtains HDF5 from its host environment and uses its runtime pinning.
The development and Python Docker images install HDF5 development files; the REST
image builds and starts `snt server`.

## Preparing a release

Archive-based recipes retain their last published version and checksums until a
new release archive is available. Do not change the version while retaining an
old checksum, or use a locally generated archive's checksum for a GitHub URL.

After publishing the intended Git tag, update all archive-based recipes together:

```sh
python3 packaging/update_release.py 0.8.4
python3 packaging/update_release.py 0.8.4 --check
```

The command downloads the GitHub tag archive, checks its `CODE_VERSION` and unified CLI support, and
computes SHA-256 and SHA-512 digests for Homebrew, vcpkg, and Conda. It updates the
vcpkg manifest version as well. It does not publish packages or modify the project
version. `--archive /path/to/downloaded.tar.gz` supports an already downloaded copy
of the same GitHub archive. Download and validation failures leave recipes unchanged.

Homebrew and vcpkg currently retain their 0.8.0 source pins pending this release step;
the published 0.8.3 archive also predates the unified CLI. The dependency and
command changes in these recipes target the next release (for example, 0.8.4). Full package
builds should be performed after refreshing those pins, rather than treating the
old archive as containing the current implementation. Conda, which packages only
the Python bindings, is updated to the published 0.8.3 archive and its verified
checksum; it does not depend on the executable unification.

## Local checks

```sh
python3 -m unittest discover -s packaging/tests
ruby -c packaging/homebrew/scinumtools3.rb
```

Before publishing, also build the recipes with their native package managers,
verify `snt --version` and `snt server --help` for CLI packages, and import
`scinumtools3` for Python packages. Docker builds require a running Docker daemon.
