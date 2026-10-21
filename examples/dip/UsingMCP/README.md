# Prepare a validated flow configuration through MCP

This example shows how an AI agent can inspect a DIPL model, propose a parameter
change, validate its consequences, and retrieve reproducible outputs. The Python
script plays the client role using the official MCP SDK. It needs no LLM, API
key, SNT Python bindings, or manual JSON-RPC messages. The server remains the
native C++ `snt mcp` executable.

The example models a channel-flow setup using `Re = density × velocity × diameter
/ viscosity`. Its DIPL constraint requires `Re < 2300`:

| Parameter | Value |
| --- | --- |
| Density | 1000 kg/m³ |
| Dynamic viscosity | 0.001 Pa·s |
| Hydraulic diameter | 0.01 m |
| Baseline velocity | 0.1 m/s → Re = 1000 |
| Rejected proposal | 0.5 m/s → Re = 5000 |
| Accepted proposal | 0.2 m/s → Re = 2000 |

This is a parameter-validation example, not a flow solver. The threshold is an
illustrative model constraint; choosing an appropriate physical model remains
the user's responsibility.

## Run it

Build SNT from this checkout with the CLI and MCP enabled (`ENABLE_SNT_MCP=ON`,
the default). The example expects the snapshot and artifact tools in this
checkout; an older installed binary may not provide them. Report support is
optional. Use Python 3.11 or newer for the client.

From the repository root:

```sh
python3 -m venv examples/dip/UsingMCP/.venv
examples/dip/UsingMCP/.venv/bin/python -m pip install -r examples/dip/UsingMCP/requirements.txt
examples/dip/UsingMCP/.venv/bin/python examples/dip/UsingMCP/demo.py --snt build/bin/snt
```

For an installed native executable, omit `--snt build/bin/snt`. The script
starts a stdio server publishing this example as `flow`, then closes the server
when finished. Python and the pinned SDK are **client-only dependencies**.

The output includes:

```text
Connected: discovered 14 tools
Parameters (paginated): fluid.density, fluid.viscosity, geometry.diameter, flow.velocity, flow.reynolds
Baseline: inlet velocity 0.1 m/s; Reynolds number 1000
Rejected 0.5 m/s: the derived Reynolds number would violate the model constraint
Accepted 0.2 m/s: Reynolds number 2000; two evaluated parameters changed
...
Source model unchanged. Outputs: .../examples/dip/UsingMCP/output
MCP demonstration passed.
```

A build without report support discovers 13 tools and skips the Markdown report.
On failure, the script exits unsuccessfully with the failed assertion or client
error. Run it without Python's `-O` flag so its verification assertions remain active.

## What the client does

1. Discovers tools, the DIPL help resource, and the named environment.
2. Creates a baseline DIPH5 snapshot, then lists all parameters using pages of two.
3. Inspects the derived Reynolds number and reads the velocity override contract.
4. Previews the invalid proposal and checks the constraint diagnostics.
5. Previews the valid proposal and saves it as an immutable candidate.
6. Compares baseline and candidate snapshots and checks that velocity and Reynolds
   number are the two changed parameters.
7. Exports the candidate to JSON, generates a Markdown report when supported,
   and downloads all artifacts in 1024-byte chunks, verifying size and SHA-256.
8. Re-inspects the source input to confirm the original Reynolds number is still 1000.

The helper functions in [demo.py](demo.py) demonstrate reusable client patterns:
`call()` handles structured errors and large-result references; `read_bytes()`
follows chunk offsets and verifies a complete artifact; `save()` retains a
server-generated file locally. Tool results are interpreted as structured data.

## Outputs you can reuse

| File in `output/` | Purpose |
| --- | --- |
| `baseline.diph5` | Original evaluated environment, including its recorded dependency graph |
| `candidate.diph5` | Accepted evaluated environment for later inspection and exchange |
| `candidate.json` | Candidate parameter values for a downstream application |
| `candidate.md` | Human-readable parameter report, when report support is enabled |
| `comparison.json` | Structured baseline-versus-candidate differences |

The output directory must be empty to avoid overwriting previous results. For
another run, supply a new directory with `--output-dir /path/to/another-run`.
The source `DIPfile` and `parameters.dip` are never modified.

Downloaded snapshots work with the existing CLI after the MCP session ends:

```sh
build/bin/snt dip describe --input examples/dip/UsingMCP/output/candidate.diph5 --path flow.reynolds
build/bin/snt dip compare examples/dip/UsingMCP/output/baseline.diph5 examples/dip/UsingMCP/output/candidate.diph5
```

The comparison command returns exit status **1** because these snapshots differ.
Server artifact identifiers expire after one hour or server shutdown, so the
client downloads them before leaving the session. To create another candidate,
apply new overrides to the source environment; saved snapshots are immutable.

## Use an existing HTTP server

Start the server in another terminal from the repository root:

```sh
build/bin/snt mcp --transport streamable-http --port 8081 --project flow=examples/dip/UsingMCP/DIPfile
```

Then connect without launching a second server:

```sh
examples/dip/UsingMCP/.venv/bin/python examples/dip/UsingMCP/demo.py \
  --url http://127.0.0.1:8081/mcp --output-dir /tmp/snt-flow-http
```

The endpoint must publish this model as `flow`, or use `--environment NAME` to
match its configured name. Add `--legacy` to either invocation to exercise MCP
initialization for legacy clients. The default uses the SDK's automatic mode.

For Docker, first build both images as described in the
[MCP integration guide](../../../docs/source/integrations/mcp.rst),
then start the same example with a read-only source mount:

```sh
docker run --rm -p 127.0.0.1:8081:8081 \
  -v "$PWD/examples/dip/UsingMCP:/data:ro" scinumtools3-mcp \
  --transport streamable-http --address 0.0.0.0 --project flow=/data/DIPfile
```

Run the Python HTTP command above against it. The image reuses the native SNT
binary from the REST image and does not require a Python server adapter.

## Try it with an AI assistant

Configure your MCP client with the native executable and arguments
`mcp --project flow=/absolute/path/to/UsingMCP/DIPfile`, as shown in the
[integration guide](../../../docs/source/integrations/mcp.rst).
A useful task to give the assistant is:

> Inspect the flow model and explain the velocity constraints. Check whether
> 0.5 m/s is allowed. If it fails, test 0.2 m/s, retain baseline and candidate
> snapshots, compare their evaluated parameters, and retrieve the candidate
> JSON and report. Keep the source files unchanged.

The script makes the same checks deterministically, so you can first verify
that the protocol, native server, and model work independently of an LLM.
