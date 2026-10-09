# `snt hub` application module

This directory is compiled into the unified `snt` executable. C++ code owns
catalogue validation, pinned Git fetch, workspace locks, and adapter
dispatch. It uses libcurl for HTTPS and the vendored nlohmann JSON header.
Git is needed for `fetch`; Python 3 provisions and runs the local adapter
when `setup`, `build`, or `run` first needs it.

The normal catalogue is `https://scinumtools.github.io/snt-hub/catalog/v1.json`.
Offline fixture tests can set `SNT_HUB_LOCAL_FIXTURE=1` and
`SNT_HUB_CATALOG_URL=file:///.../catalogue.json` to use local Git repositories.
Those variables are for controlled tests; ordinary fetch accepts HTTPS
catalogue and repository URLs.

`ctest -R cli.hub` runs the offline command test. The opt-in
`tests/test_external_workspace_local.py` exercises a clean local Hub checkout
and a selected adapter when `SNT_HUB_TEST_ROOT`, `SNT_HUB_TEST_PROJECT`, and
`SNT_EXECUTABLE` are supplied. Set `SNT_HUB_TEST_SETUP` to choose a recipe.

Optional `build` and `run` dispatch is available only when the pinned project
record and catalogue both declare matching version-1 capabilities. The
project's adapter owns compiler and solver commands. A build adapter writes
`build-lock.json` with `executable` and `build_log` paths, and any build
command details it can report. SNT confines those paths to the staged build,
normalizes them to relative paths before publishing, and adds revision and
SHA-256 provenance. A run adapter can add command and launcher details to
`run-lock.json`; SNT adds executable identity and exit status even when the
adapter fails. Projects must opt in with reviewed recipes.
