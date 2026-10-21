MCP: connecting AI agents
=========================

``snt mcp`` provides an AI-ready interface to DIPL and PUQ through the
Model Context Protocol (MCP). Agents can discover parameters, understand units
and constraints, validate proposed changes, create immutable evaluated
candidates, compare them, and retrieve exports and reports. These operations
use the same C++ APIs as the :doc:`CLI <cli>` and :doc:`REST server <rest>`.
Scientific review remains necessary when choosing a model and its parameters.

The server is native C++, with no Python or MCP SDK runtime dependency.
It uses cpp-httplib for HTTP and nlohmann/json for JSON. Tool execution runs
in supervised workers of the same ``snt`` executable so cancellation and time
limits do not require changes to the scientific core. Python is used only
by the optional example client and tests.

Connect a client
----------------

MCP is enabled in CLI builds by default (``ENABLE_SNT_MCP=ON``). Build from
this checkout following :doc:`../installation`, then start a stdio service:

.. code-block:: bash

   build/bin/snt mcp --project flow=examples/dip/UsingMCP/DIPfile

The process waits for newline-delimited JSON-RPC on stdin and writes protocol
responses to stdout. Normally an MCP client starts and owns this process.
A client configuration using absolute paths can look like this:

.. code-block:: json

   {
     "mcpServers": {
       "snt3": {
         "command": "/absolute/path/to/snt",
         "args": ["mcp", "--project", "flow=/absolute/path/to/model/DIPfile"]
       }
     }
   }

Client configuration formats vary; supply the executable and argument vector
through your client's MCP server settings. Repeated ``--project NAME=PATH``
arguments expose multiple inputs. Inputs may be DIPfiles, DIPL sources, or
saved ``.diph5`` environments. ``--input PATH`` publishes an input as
``default``, which is also the default ``environment`` tool argument.

Available operations
--------------------

Tools advertise descriptions, argument schemas, output schemas, and read-only
annotations. Read-only means that operations do not edit the source project;
snapshots, reports, and other generated results use temporary server storage.
Clients should discover tools at runtime because report support is optional.

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Tool
     - Purpose
   * - ``dipl_environments``
     - Discover configured inputs, input kinds, and supported operations.
   * - ``dipl_list``
     - Search parameters by path and tags, with bounded values and continuation cursors.
   * - ``dipl_describe``
     - Inspect values, units, constraints, metadata, provenance, and recorded dependencies.
   * - ``dipl_schemas`` / ``dipl_sources``
     - Inspect schema information and source metadata, including fingerprints.
   * - ``dipl_override_contract``
     - Understand an override target before proposing changes.
   * - ``dipl_preview``
     - Validate overrides and inspect their effects without publishing a candidate.
   * - ``dipl_evaluate``
     - Evaluate inline DIPL and return a paginated parameter listing.
   * - ``dipl_snapshot``
     - Validate inputs with optional overrides and retain an immutable DIPH5 candidate.
   * - ``dipl_compare``
     - Compare two saved or generated snapshots, returning structured counts and differences.
   * - ``dipl_export``
     - Generate C++, C, Fortran, Rust, Julia, JSON, JSON with metadata, or YAML parameters.
   * - ``dipl_report``
     - Generate Markdown, HTML, reStructuredText, LaTeX, Typst, text, or JSON reports when built with report support.
   * - ``artifact_read``
     - Retrieve generated output in base64 chunks with size and SHA-256 metadata.
   * - ``puq``
     - Evaluate or convert quantities, inspect them, and list unit definitions.

Resources include ``snt://environments`` (the configured input catalogue),
``snt://help/dipl`` (syntax and workflow guidance), and
``snt://artifacts/{id}{?offset}`` (bounded binary reads of generated artifacts).
The artifact resource returns ``blob`` content and an ``_meta.next_uri`` when
more bytes remain. Artifact identifiers are returned by tools rather than
being added to the static resource catalogue.

Validate and retain a candidate
-------------------------------

A useful agent workflow is:

1. Discover the environment and read ``snt://help/dipl``.
2. Use ``dipl_list``, ``dipl_describe``, and ``dipl_override_contract`` to
   understand the target and its dependencies.
3. Create a baseline with ``dipl_snapshot`` and retain its ``environment`` identifier.
4. Preview an override against the source input. For example:

   .. code-block:: json

      {
        "environment": "flow",
        "overrides": ["flow.velocity = 0.2 m/s"]
      }

5. Check ``candidate_valid`` and diagnostics. A rejected preview is a successful
   tool call describing an invalid candidate, rather than a transport failure.
6. Call ``dipl_snapshot`` against the source with the accepted overrides. Pass
   the returned ``environment`` identifier to subsequent inspection, comparison,
   export, and report tools.
7. Compare the baseline and candidate identifiers with ``dipl_compare``;
   download the generated files before the service session ends.

Overrides are unwrapped DIPL bodies. They use the existing parser and
recalculate dependent values. They never write back to the source project.
``dipl_export`` and ``dipl_report`` also accept overrides directly against
source inputs. For consistency across several operations, create a snapshot
first and use its identifier for each operation.

Source-based calls read current files independently: previewing does not freeze
inputs for a later snapshot. A snapshot freezes the successfully evaluated
candidate, including its dependency graph and source fingerprints. It remains
inspectable if the source files subsequently change. Saved snapshots cannot
be re-evaluated with nonempty overrides and do not restore executable schema
definitions. Start a new candidate from source when further changes are needed.
For source inspection, request ``record_dependencies=true`` when the graph is needed.

Comparisons return ``scope``, ``equal``, ``added``, ``removed``, ``changed``,
``differences``, and ``omitted``, plus a formatted ``result`` summary. Each
reported difference identifies a path, category, changed fields, before/after
summaries, and array changes. ``max_details`` and ``max_array_examples`` bound
details while counts still describe the complete comparison.

Runnable example
----------------

The `UsingMCP example
<https://github.com/scinumtools/snt3/tree/main/examples/dip/UsingMCP>`_
uses the official Python MCP client to prepare a channel-flow candidate. It
rejects a velocity that violates a derived Reynolds-number constraint, accepts
a valid alternative, compares snapshots, and downloads JSON, DIPH5, and an
optional Markdown report. It verifies downloaded bytes using SHA-256 and
checks that the original model is unchanged. No LLM or API key is required.

From the repository root, with Python 3.11 or newer:

.. code-block:: bash

   python3 -m venv examples/dip/UsingMCP/.venv
   examples/dip/UsingMCP/.venv/bin/python -m pip install -r examples/dip/UsingMCP/requirements.txt
   examples/dip/UsingMCP/.venv/bin/python examples/dip/UsingMCP/demo.py --snt build/bin/snt

See the example README for expected output, downloaded files, legacy-protocol
mode, and connecting to an existing HTTP server.

Results, errors, and execution limits
-------------------------------------

``dipl_list`` and ``dipl_evaluate`` return ``next_cursor`` when more parameters
remain. Continue with ``dipl_list(cursor=..., limit=...)`` until it is null.
A continuation uses the stored listing, even if the original source changes.
``limit=0`` gives counts; use a positive limit to advance. Page limits are at
most 1000. The initial call materializes the matching listing internally;
pagination bounds responses, not total evaluation memory.

Exports and reports include ``artifact`` metadata and an ``inline`` flag.
Outputs up to 128 KiB also include ``content``. Larger structured tool results
return ``result_ref`` pointing to complete JSON. For either kind of artifact,
call ``artifact_read`` with its ``id`` and successive ``offset`` values, append
the decoded bytes, and stop at ``next_offset=null``. The maximum decoded chunk
is 128 KiB. Check the complete file's ``size`` and ``sha256``.

Artifacts expire after one hour and are removed when the server exits. They
are shared by clients of that server process and are not portable identifiers.
Download DIPH5 files to retain candidates for another session. Published
artifacts and worker staging each have a 256 MiB storage budget; staging also
contains temporary copies. The protocol message limit is 4 MiB and inline DIPL
is limited to 262144 characters. Full stores reject new artifacts; expiry or a
server restart releases them.

Tool failures set ``isError=true`` and provide ``structuredContent.error`` with
``code``, ``message``, ``details``, ``suggestion``, ``location``, and ``retryable``.
Locations and hints depend on the underlying diagnostic. For example,
``argument.invalid`` calls for corrected arguments, ``artifact.not_found``
requires regenerating an output, and ``server.busy`` permits retrying later.
Malformed protocol requests instead return JSON-RPC errors.

The server executes one tool at a time. Overlapping calls receive
``server.busy``; discovery and ping remain available. Each tool has a 30-second
wall-time limit by default, configurable with ``--timeout-ms`` from 1 to
3600000. Expiry returns ``execution.timeout`` and terminates the native worker.
Stdio cancellation uses ``notifications/cancelled`` with the matching request
identifier; HTTP cancellation uses connection closure. Cancelled calls publish
no generated artifacts and send no tool response. Workers use the existing
C++ APIs directly, without shell commands or CLI output parsing.

HTTP and Docker
---------------

.. code-block:: bash

   snt mcp --transport streamable-http --port 8081 --project flow=examples/dip/UsingMCP/DIPfile

Connect to ``http://127.0.0.1:8081/mcp``. This endpoint carries MCP JSON-RPC;
ordinary HTTP integrations can continue using :doc:`REST <rest>` separately.
The adapters reuse the native API, comparison serialization, temporary-directory
helper, and cpp-httplib dependency setup.

The Docker image reuses the executable built by the REST image:

.. code-block:: bash

   docker build -f packaging/docker/rest/Dockerfile -t scinumtools3-rest .
   docker build -f packaging/docker/mcp/Dockerfile -t scinumtools3-mcp .
   docker run --rm -p 127.0.0.1:8081:8081 \
     -v "$PWD/examples/dip/UsingMCP:/data:ro" scinumtools3-mcp \
     --transport streamable-http --address 0.0.0.0 --project flow=/data/DIPfile

For a client-managed container using stdio, use ``docker run --rm -i`` with
the same mount and ``--transport stdio --project flow=/data/DIPfile``. Do not
allocate a TTY. Docker arguments replace the image's default command, so
include transport and bind address when configuring an HTTP service.

The service is intended for trusted clients and projects. Named environments
are not a filesystem sandbox: DIPL may read referenced files with the service
user's permissions. HTTP has no built-in authentication or client isolation;
use loopback or an authenticated reverse proxy. Browser ``Origin`` headers
must match the configured loopback origins or an explicit
``--allow-origin https://your-client.example``. The image runs as a non-root
user; mount only the data it should access.

Protocol coverage and tests
---------------------------

The adapter supports MCP ``2026-07-28`` discovery and per-request metadata,
plus ``2025-11-25`` and ``2025-06-18`` initialization. HTTP uses stateless JSON
responses to ``POST /mcp``; ``GET`` and ``DELETE`` return 405. Persistent SSE
streams, server-initiated requests, subscriptions, prompts, source editing,
PDF compilation, and host callback registration are not exposed.

Run the native integration suite with a standard-library Python client:

.. code-block:: bash

   python3 apps/snt-mcp/tests/test_mcp.py build/bin/snt
   SNT_MCP_TEST_HTTP=1 python3 apps/snt-mcp/tests/test_mcp.py build/bin/snt

The HTTP test requires permission to bind a loopback socket. With
``ENABLE_UNIT_TESTS=ON``, CTest also registers ``cli.mcp`` and ``cli.mcp.help``.
