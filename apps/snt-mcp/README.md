# Native SNT3 MCP integration

`snt mcp` exposes existing DIPL and PUQ APIs to AI agents through MCP.
It is native C++, reuses cpp-httplib and nlohmann/json, and requires no Python
runtime or MCP SDK. The scientific core is unchanged. Each tool runs in a
supervised worker of the same native executable, enabling cancellation and
wall-time limits without parser hooks or shell commands.

The full user guide is the dedicated
[MCP integration page](../../docs/source/integrations/mcp.rst). It covers client
configuration, every tool, immutable candidates, artifact retrieval, structured
errors, HTTP, Docker, and operational limits.

Build the CLI with `ENABLE_SNT_MCP=ON` (the default), then let an MCP client run:

```sh
build/bin/snt mcp --project flow=examples/dip/UsingMCP/DIPfile
```

The default transport is stdio. For an HTTP client:

```sh
build/bin/snt mcp --transport streamable-http --project flow=examples/dip/UsingMCP/DIPfile
```

Connect to `http://127.0.0.1:8081/mcp`. Multiple `--project NAME=PATH` arguments
publish multiple sources or saved DIPH5 inputs. `--input PATH` names an input
`default`. HTTP is intended for trusted clients; it has no built-in authentication.

Try [examples/dip/UsingMCP](../../examples/dip/UsingMCP) for a practical,
verified workflow using the official Python SDK **as a client only**. It checks
a derived constraint, creates snapshots, compares changes, and downloads JSON,
DIPH5, and a report without changing the original model.

## Implementation

- `tools.cpp` adapts existing native APIs; `contracts.cpp` supplies output
  schemas, argument descriptions, structured errors, and a DIPL help resource.
- `protocol.cpp` handles JSON-RPC and MCP versions; `transports.cpp` handles
  stdio and Streamable HTTP with cpp-httplib.
- `execution.cpp` supervises one native worker at a time, with a default
  30-second deadline (`--timeout-ms`) and cancellation.
- `storage.cpp` stages immutable snapshots, exports, reports, and paginated
  listings. Artifacts expire after one hour or server shutdown; the session
  artifact budget is 256 MiB and chunk reads are at most 128 KiB.
- REST and MCP share the native API layer, comparison serialization,
  temporary-directory helper, and HTTP dependency setup. Their wire protocols
  and entry points remain independent.

The server supports MCP `2026-07-28` discovery plus `2025-11-25` and `2025-06-18`
initialization. It exposes 14 tools when reports are enabled, 13 otherwise.
Source editing, PDF compilation, executable callbacks, subscriptions, persistent
SSE streams, and server-initiated requests are not exposed.

## Verification

Standard-library protocol tests exercise the native executable without an SDK:

```sh
python3 apps/snt-mcp/tests/test_mcp.py build/bin/snt
SNT_MCP_TEST_HTTP=1 python3 apps/snt-mcp/tests/test_mcp.py build/bin/snt
```

The second command needs loopback socket access. With `ENABLE_UNIT_TESTS=ON`,
CTest registers `cli.mcp` and `cli.mcp.help`. The UsingMCP example additionally
checks interoperability with the official SDK and its output-schema validation.
