# `snt hub` application module

This directory is compiled into the unified `snt` executable. C++ code owns
catalogue validation, pinned Git installation, lock files, and adapter
dispatch. It uses libcurl for HTTPS and the vendored nlohmann JSON header.
Git and Python 3 are subprocess prerequisites for `install`; Python runs only
the isolated project adapter after installation.

The normal catalogue is `https://scinumtools.github.io/snt-hub/catalog/v1.json`.
Offline fixture tests can set `SNT_HUB_LOCAL_FIXTURE=1` and
`SNT_HUB_CATALOG_URL=file:///.../catalogue.json` to use local Git repositories.
Those variables are for controlled tests; ordinary installation accepts HTTPS
catalogue and repository URLs.

`ctest -R cli.hub` runs the offline command test. The opt-in
`tests/test_arepo_install_local.py` and `tests/test_arepo_local.py` exercise a
clean local SNT Hub checkout and its Arepo adapter when `SNT_HUB_AREPO_ROOT`
and `SNT_EXECUTABLE` are supplied.
