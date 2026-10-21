"""Native MCP integration tests. Python standard library only; no runtime SDK."""

from concurrent.futures import ThreadPoolExecutor
import base64
import hashlib
import json
import os
from pathlib import Path
import queue
import socket
import subprocess
import sys
import tempfile
import threading
import time
import unittest
from urllib.error import HTTPError
from urllib.request import Request, urlopen

SNT = str(Path(sys.argv.pop(1)).resolve())
VERSION = "2026-07-28"
META = {"io.modelcontextprotocol/protocolVersion": VERSION,
        "io.modelcontextprotocol/clientCapabilities": {},
        "io.modelcontextprotocol/clientInfo": {"name": "snt-test", "version": "1"}}
SOURCE = '''$schema record
  count int = 1
records list : record
records[]
simulation
  distance float = 24 m
  duration float = 8 s
    !condition ({.} > 0 s)
  speed float = ({?simulation.distance} / {?simulation.duration}) m/s
    !tags ["derived", "physics"]
    ?descr "Average speed"
'''


def message(method, params=None, modern=True, request_id=1):
    params = dict(params or {})
    if modern:
        params["_meta"] = META.copy()
    return {"jsonrpc": "2.0", "id": request_id, "method": method, "params": params}


class Stdio:
    def __init__(self, args):
        self.log = tempfile.TemporaryFile()
        env = dict(os.environ, SNT_MCP_PYTHON="/this/interpreter/does/not/exist")
        # Prove the native server needs no executables on PATH. Windows may
        # require PATH to locate the build's HDF5 runtime DLLs.
        if os.name != "nt":
            env["PATH"] = ""
        self.process = subprocess.Popen([SNT, "mcp", *args], env=env,
                                        stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                        stderr=self.log, text=True, encoding="utf-8")
        self.lines = queue.Queue()
        def read():
            for line in self.process.stdout:
                self.lines.put(line)
            self.lines.put(None)
        self.reader = threading.Thread(target=read, daemon=True)
        self.reader.start()

    def send(self, value):
        self.process.stdin.write(value + "\n")
        self.process.stdin.flush()

    def request(self, value):
        self.send(json.dumps(value) if not isinstance(value, str) else value)
        line = self.lines.get(timeout=40)
        if line is None:
            self.log.seek(0)
            raise AssertionError(self.log.read().decode())
        return json.loads(line)

    def close(self):
        self.process.stdin.close()
        try:
            self.process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            self.process.kill()
            self.process.wait()
        self.reader.join(timeout=2)
        self.process.stdout.close()
        self.log.close()


class Integration(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory(prefix="snt-native-mcp test ")
        self.addCleanup(self.directory.cleanup)
        root = Path(self.directory.name).resolve()
        self.source = root / "parameters.dip"
        self.source.write_text(SOURCE, encoding="utf-8")
        self.project = root / "DIPfile"
        self.project.write_text('code[]\n  file = "parameters.dip"\n', encoding="utf-8")
        self.before, self.after = root / "before.diph5", root / "after.diph5"
        for path, override in ((self.before, []), (self.after, ["-i", "override_string", "simulation.distance = 48 m"])):
            subprocess.run([SNT, "dip", "parse", "--project", str(self.project), *override,
                            "--record-graph", "--save", str(path)], check=True, capture_output=True)
        self.args = ["--input", str(self.project), "--project", f"before={self.before}",
                     "--project", f"after={self.after}"]
        self.client = Stdio(self.args)
        self.addCleanup(self.client.close)

    def call(self, tool, **arguments):
        response = self.client.request(message("tools/call", {"name": tool, "arguments": arguments}))
        self.assertNotIn("error", response, response)
        result = response["result"]
        self.assertFalse(result.get("isError"), result)
        self.assertEqual(result["resultType"], "complete")
        self.assertEqual(json.loads(result["content"][0]["text"]), result["structuredContent"])
        return result["structuredContent"]

    def test_discovery_and_semantic_workflow(self):
        discovered = self.client.request(message("server/discover"))["result"]
        self.assertIn(VERSION, discovered["supportedVersions"])
        self.assertEqual(discovered["ttlMs"], 0)
        self.assertEqual(discovered["cacheScope"], "private")
        tools = self.client.request(message("tools/list"))["result"]["tools"]
        names = {tool["name"] for tool in tools}
        self.assertTrue({"dipl_describe", "dipl_preview", "puq"} <= names)
        self.assertTrue(all(tool["annotations"]["readOnlyHint"] for tool in tools))
        catalog = self.call("dipl_environments")
        self.assertEqual({item["name"] for item in catalog["environments"]}, {"default", "before", "after"})
        resource = self.client.request(message("resources/read", {"uri": "snt://environments"}))["result"]
        self.assertEqual(json.loads(resource["contents"][0]["text"]), catalog)
        described = self.call("dipl_describe", path="simulation.speed", record_dependencies=True)
        expected = subprocess.check_output([SNT, "dip", "describe", "--input", str(self.project),
                                            "--path", "simulation.speed", "--record-graph"], text=True)
        self.assertEqual(described, json.loads(expected))
        self.assertEqual(described["value"]["text"], "3")
        self.assertTrue(described["dependencies"]["available"])
        selected = self.call("dipl_list", query="?simulation.", tags_all=["derived"], limit=1)
        self.assertEqual([item["path"] for item in selected["items"]], ["simulation.speed"])
        self.assertTrue(self.call("dipl_list", limit=1)["truncated"])
        self.assertEqual(self.call("dipl_list", query="?missing")["total"], 0)
        self.assertIn("record", json.dumps(self.call("dipl_schemas")))
        self.assertIn("parameters.dip", json.dumps(self.call("dipl_sources")))
        self.assertEqual(self.call("dipl_override_contract", path="simulation.distance")["kind"], "existing_value")
        valid = self.call("dipl_preview", overrides=["simulation.distance = 48 m"])
        self.assertTrue(valid["candidate_valid"])
        self.assertEqual(valid["diff"]["changed"], 2)
        invalid = self.call("dipl_preview", overrides=["simulation.duration = 0 s"])
        self.assertFalse(invalid["candidate_valid"])
        self.assertTrue(invalid["candidate_diagnostics"])
        baseline = self.call("dipl_preview", overrides=[])
        self.assertTrue(baseline["baseline_valid"])
        self.assertEqual(baseline["diff"]["total"], 0)
        self.assertEqual(self.source.read_text(encoding="utf-8"), SOURCE)
        self.assertEqual(self.call("dipl_evaluate", source="answer int = 42")["items"][0]["value"]["text"], "42")
        self.assertIn("$(echo injected)", json.dumps(self.call("dipl_evaluate", source='label str = "$(echo injected)"')))
        self.assertFalse(self.call("dipl_describe", environment="before", path="simulation.speed")["source_text"]["available"])
        self.assertFalse(self.call("dipl_compare", before="before", after="after")["equal"])
        self.assertTrue(self.call("dipl_compare", before="before", after="before")["equal"])
        self.assertIsInstance(json.loads(self.call("dipl_export", format="json")["content"]), dict)
        if "dipl_report" in names:
            self.assertIn("Average speed", self.call("dipl_report", format="md")["content"])
        self.assertEqual(self.call("puq", operation="convert", expression="2*m", output_units="cm")["result"], "200*cm")
        self.assertEqual(self.call("puq", operation="eval", expression="-2*cm")["result"], "-2*cm")
        self.call("puq", operation="info", expression="2*kg")
        self.call("puq", operation="list", category="sys")

    def test_tool_errors_are_actionable(self):
        for tool, args, expected in [
            ("dipl_list", {"environment": "../../secret"}, "Unknown environment"),
            ("dipl_list", {"limit": -1}, "between 0 and 1000"),
            ("dipl_list", {"limit": 1001}, "between 0 and 1000"),
            ("dipl_list", {"limit": True}, "must be integer"),
            ("dipl_list", {"unexpected": 1}, "Unknown argument"),
            ("dipl_describe", {}, "Missing argument"),
            ("dipl_evaluate", {"source": "broken nonsense = ("}, "Unbalanced parentheses"),
            ("puq", {"operation": "convert", "expression": "2*m", "output_units": "s"}, "Incompatible dimensions"),
            ("puq", {"operation": "eval", "expression": "invalid_unit_xyz"}, "Unknown unit base"),
        ]:
            with self.subTest(tool=tool, args=args):
                result = self.client.request(message("tools/call", {"name": tool, "arguments": args}))["result"]
                self.assertTrue(result["isError"])
                self.assertIn(expected, result["content"][0]["text"])

    def test_exports_reports_with_overrides(self):
        self.client = Stdio([*self.args, "--project", f"source={self.source}"])
        self.addCleanup(self.client.close)
        tools = {tool["name"]: tool for tool in self.client.request(message("tools/list"))["result"]["tools"]}
        available = ["dipl_export"] + (["dipl_report"] if "dipl_report" in tools else [])
        originals = {path: path.read_bytes() for path in self.source.parent.iterdir()}
        overrides = ["simulation.distance = 48 m", "simulation.duration = 4 s"]
        for tool in available:
            self.assertEqual(tools[tool]["inputSchema"]["properties"]["overrides"]["default"], [])
            format = "json" if tool == "dipl_export" else "md"
            def values(content):
                # Reports also contain per-evaluation source identifiers.
                return json.loads(content) if format == "json" else [
                    line for line in content.splitlines() if line.startswith("| [")]
            for environment in ("default", "source"):
                baseline = self.call(tool, environment=environment, format=format)["content"]
                self.assertEqual(values(baseline), values(self.call(tool, environment=environment, format=format, overrides=[])["content"]))
                candidate = self.call(tool, environment=environment, format=format, overrides=overrides)["content"]
                if tool == "dipl_export":
                    output = self.source.parent / "expected.json"
                    subprocess.run([SNT, "dip", "parse", "--project", str(self.project),
                                    "-i", "override_string", overrides[0], "-i", "override_string", overrides[1],
                                    "--generate", "json", str(output)], check=True, capture_output=True)
                    self.assertEqual(json.loads(candidate), json.loads(output.read_text()))
                    self.assertNotEqual(json.loads(candidate), json.loads(baseline))
                    output.unlink()
                else:
                    self.assertRegex(candidate, r"\[simulation.distance\].*\| 48 m \|")
                    self.assertRegex(candidate, r"\[simulation.speed\].*\| 12 m")
                    self.assertIn("Reads during evaluation", candidate)
                self.assertEqual(values(baseline), values(self.call(tool, environment=environment, format=format)["content"]))
            # Snapshots remain exportable, but cannot be re-evaluated with overrides.
            self.call(tool, environment="before", format=format, overrides=[])
            for arguments in ({"overrides": ["simulation.duration = 0 s"]},
                              {"environment": "before", "overrides": overrides},
                              {"overrides": "simulation.distance = 48 m"},
                              {"overrides": [7]}):
                result = self.client.request(message("tools/call", {"name": tool, "arguments": arguments}))["result"]
                self.assertTrue(result["isError"], result)
                self.assertIn("error", result["structuredContent"])
                if arguments.get("environment") == "before":
                    self.assertIn("Overrides require DIPL source input", result["content"][0]["text"])
        self.assertEqual(originals, {path: path.read_bytes() for path in self.source.parent.iterdir()})

    def test_structured_comparisons(self):
        for path, source in ((self.before, 'keep int = 1\nremove int = 2\narray int[4] = [1,2,3,4]\n'),
                             (self.after, 'keep int = 1\nadd int = 3\narray int[4] = [1,9,3,8]\n')):
            subprocess.run([SNT, "dip", "parse", "-i", "string", source, "--save", str(path)],
                           check=True, capture_output=True)
        result = self.call("dipl_compare", before="before", after="after", max_array_examples=1)
        self.assertEqual((result["scope"], result["equal"], result["added"], result["removed"], result["changed"]),
                         ("effective", False, 1, 1, 1))
        self.assertEqual(result["omitted"], 0)
        differences = {item["path"]: item for item in result["differences"]}
        self.assertEqual(differences["add"]["kind"], "added")
        self.assertEqual(differences["remove"]["kind"], "removed")
        array = differences["array"]
        self.assertEqual((array["kind"], array["category"], array["changed_elements"], array["example_indices"]),
                         ("changed", "value", 2, [1]))
        self.assertTrue(array["fields"])
        self.assertNotEqual(array["before"], array["after"])
        self.assertIn("1 added, 1 removed, 1 changed", result["result"])
        bounded = self.call("dipl_compare", before="before", after="after", max_details=1)
        self.assertEqual((len(bounded["differences"]), bounded["omitted"]), (1, 2))
        counts = self.call("dipl_compare", before="before", after="after", max_details=0)
        self.assertEqual((counts["differences"], counts["omitted"], counts["changed"]), ([], 3, 1))
        full = self.call("dipl_compare", before="before", after="after", scope="full", max_array_examples=0)
        self.assertEqual(full["scope"], "full")
        self.assertTrue(all(not item["example_indices"] for item in full["differences"]))
        equal = self.call("dipl_compare", before="before", after="before", scope="full")
        self.assertEqual((equal["equal"], equal["differences"], equal["omitted"]), (True, [], 0))

    def test_protocol_errors_notifications_and_limits(self):
        for raw, code in [("{", -32700), ("[]", -32600), ("null", -32600),
                          (message("ping", request_id=None), -32600),
                          (message("tools/list", {"cursor": "invalid"}), -32602),
                          (message("no-such-method"), -32601),
                          (message("tools/call", {"name": "missing"}), -32602),
                          (message("resources/read", {"uri": "file:///secret"}), -32602)]:
            with self.subTest(raw=raw):
                self.assertEqual(self.client.request(raw)["error"]["code"], code)
        request = message("server/discover")
        request["params"]["_meta"]["io.modelcontextprotocol/protocolVersion"] = "2099-01-01"
        error = self.client.request(request)["error"]
        self.assertEqual(error["code"], -32022)
        self.assertIn(VERSION, error["data"]["supported"])
        self.client.send(json.dumps({"jsonrpc": "2.0", "method": "notifications/cancelled", "params": {"requestId": 42}}))
        ping = self.client.request(message("ping", request_id="after-notification"))
        self.assertEqual(ping["id"], "after-notification")
        self.assertEqual(self.client.request("[" * 100 + "]" * 100)["error"]["code"], -32700)
        self.assertEqual(self.client.request(" " * (4 * 1024 * 1024 + 1))["error"]["code"], -32600)
        self.assertIn("result", self.client.request(message("ping")))

    def test_legacy_initialization(self):
        self.assertIn("error", self.client.request(message("tools/list", modern=False)))
        init = self.client.request(message("initialize", {"protocolVersion": "2025-11-25", "capabilities": {},
                                                        "clientInfo": {"name": "test", "version": "1"}}, modern=False))
        self.assertEqual(init["result"]["protocolVersion"], "2025-11-25")
        self.client.send('{"jsonrpc":"2.0","method":"notifications/initialized"}')
        response = self.client.request(message("tools/list", modern=False))
        self.assertIn("tools", response["result"])
        self.assertNotIn("resultType", response["result"])

    def read_artifact(self, artifact):
        chunks = bytearray()
        offset = 0
        while True:
            chunk = self.call("artifact_read", id=artifact["id"], offset=offset, length=131072)
            self.assertEqual(chunk["offset"], len(chunks))
            data = base64.b64decode(chunk["data"], validate=True)
            self.assertEqual(len(data), chunk["length"])
            chunks.extend(data)
            if chunk["next_offset"] is None:
                break
            self.assertGreater(chunk["next_offset"], offset)
            offset = chunk["next_offset"]
        self.assertEqual(len(chunks), artifact["size"])
        self.assertEqual(hashlib.sha256(chunks).hexdigest(), artifact["sha256"])
        return bytes(chunks)

    def test_snapshots_artifacts_and_structured_errors(self):
        tools = self.client.request(message("tools/list"))["result"]["tools"]
        self.assertTrue(all("outputSchema" in tool for tool in tools))
        self.assertTrue(all("description" in prop for tool in tools
                            for prop in tool["inputSchema"]["properties"].values()))
        help = self.client.request(message("resources/read", {"uri": "snt://help/dipl"}))["result"]
        self.assertIn("dipl_snapshot", help["contents"][0]["text"])
        baseline = self.call("dipl_snapshot")
        candidate = self.call("dipl_snapshot", overrides=["simulation.distance = 48 m"])
        identifier = candidate["environment"]
        self.source.write_text("unrelated int = 7\n", encoding="utf-8")
        described = self.call("dipl_describe", environment=identifier, path="simulation.speed")
        self.assertEqual(described["value"]["text"], "6")
        self.assertTrue(described["dependencies"]["available"])
        diff = self.call("dipl_compare", before=baseline["environment"], after=identifier)
        self.assertEqual(diff["changed"], 2)
        exported = self.call("dipl_export", environment=identifier, format="json")
        self.assertEqual(exported["artifact"]["mimeType"], "application/json")
        self.assertEqual(json.loads(self.read_artifact(exported["artifact"]))["simulation"]["speed"], 6)
        saved = self.source.parent / "downloaded.diph5"
        saved.write_bytes(self.read_artifact(candidate["artifact"]))
        inspected = subprocess.check_output([SNT, "dip", "describe", "--input", str(saved),
                                             "--path", "simulation.speed"], text=True)
        self.assertEqual(json.loads(inspected)["value"]["text"], "6")
        resource = self.client.request(message("resources/read", {"uri": candidate["artifact"]["uri"]}))["result"]
        self.assertEqual(base64.b64decode(resource["contents"][0]["blob"]), saved.read_bytes())
        for name, args, code in [
            ("artifact_read", {"id": "../../secret"}, "artifact.not_found"),
            ("artifact_read", {"id": "artifact-0-0"}, "artifact.not_found"),
            ("artifact_read", {"id": identifier, "offset": candidate["artifact"]["size"] + 1}, "argument.invalid"),
            ("artifact_read", {"id": identifier, "length": 0}, "argument.invalid"),
            ("dipl_list", {"cursor": "malformed"}, "cursor.invalid"),
            ("dipl_snapshot", {"environment": identifier, "overrides": ["simulation.distance = 1 m"]}, "argument.invalid"),
        ]:
            result = self.client.request(message("tools/call", {"name": name, "arguments": args}))["result"]
            self.assertTrue(result["isError"], result)
            error = result["structuredContent"]["error"]
            self.assertEqual(error["code"], code)
            self.assertFalse(error["retryable"])
            self.assertTrue({"message", "details", "suggestion", "location"} <= error.keys())

    def test_frozen_pagination_and_large_results(self):
        source = "".join(f"parameter{i} int = {i}\n" for i in range(1105))
        self.source.write_text(source, encoding="utf-8")
        first = self.call("dipl_list", limit=1000)
        self.assertEqual(len(first["items"]), 1000)
        self.assertEqual(first["total"], 1105)
        self.assertIsNotNone(first["next_cursor"])
        self.source.write_text("replacement int = 0\n", encoding="utf-8")
        second = self.call("dipl_list", cursor=first["next_cursor"], limit=1000)
        self.assertEqual(len(second["items"]), 105)
        self.assertEqual(second["total"], 1105)
        self.assertIsNone(second["next_cursor"])
        paths = [item["path"] for item in first["items"] + second["items"]]
        self.assertEqual(len(set(paths)), 1105)
        self.assertNotIn("replacement", paths)
        self.assertEqual(self.call("dipl_list")["total"], 1)
        # A single large metadata value exercises result references without
        # coupling the test to the cost of parsing thousands of large records.
        description = "x" * 1500000
        self.source.write_text(f'parameter int = 42\n  ?descr "{description}"\n', encoding="utf-8")
        result = self.call("dipl_list")
        self.assertIn("result_ref", result)
        listing = json.loads(self.read_artifact(result["result_ref"]))
        self.assertEqual(listing["items"][0]["path"], "parameter")
        self.assertIn(description, json.dumps(listing))
        # A smaller input still crosses the inline-export threshold. Avoid
        # re-evaluating the 1.5 MiB description just to exercise that branch.
        payload = "y" * 150000
        self.source.write_text(f'payload str = "{payload}"\n', encoding="utf-8")
        exported = self.call("dipl_export", format="json")
        self.assertFalse(exported["inline"])
        self.assertNotIn("content", exported)
        self.assertEqual(json.loads(self.read_artifact(exported["artifact"]))["payload"], payload)
        resource = self.client.request(message("resources/read", {"uri": exported["artifact"]["uri"]}))["result"]
        self.assertEqual(len(base64.b64decode(resource["contents"][0]["blob"])), 131072)
        self.assertIsNotNone(resource["contents"][0]["_meta"]["next_uri"])

    def test_cancellation_timeout_and_recovery(self):
        self.source.write_text("".join(f"parameter{i} int = {i}\n" for i in range(25000)), encoding="utf-8")
        self.client.send(json.dumps(message("tools/call", {"name": "dipl_snapshot"}, request_id="cancel-me")))
        self.assertEqual(self.client.request(message("ping", request_id="alive"))["id"], "alive")
        self.client.send(json.dumps({"jsonrpc": "2.0", "method": "notifications/cancelled",
                                     "params": {"requestId": "cancel-me"}}))
        for _ in range(100):
            response = self.client.request(message("tools/call", {"name": "puq", "arguments": {
                "operation": "eval", "expression": "2*m"}}, request_id="recovered"))
            self.assertEqual(response["id"], "recovered", "Cancelled request must not send a response")
            result = response["result"]
            if not result["isError"]:
                break
            self.assertEqual(result["structuredContent"]["error"]["code"], "server.busy")
            time.sleep(.01)
        self.assertFalse(result["isError"])
        self.assertEqual(result["structuredContent"]["result"], "2*m")
        timed = Stdio([*self.args, "--timeout-ms", "1"])
        self.addCleanup(timed.close)
        response = timed.request(message("tools/call", {"name": "dipl_snapshot"}))["result"]
        self.assertTrue(response["isError"])
        self.assertEqual(response["structuredContent"]["error"]["code"], "execution.timeout")
        self.assertIn("result", timed.request(message("ping")))

    def test_cli_errors(self):
        for options in (["--port", "0"], ["--transport", "wrong"], ["--project", "bad"],
                        ["--input", "missing"], ["--project", f"default={self.project}", "--input", str(self.project)],
                        ["--allow-origin", "*"], ["--port"]):
            result = subprocess.run([SNT, "mcp", *options], capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(result.stdout, "")

    @unittest.skipUnless(os.environ.get("SNT_MCP_TEST_HTTP"), "set SNT_MCP_TEST_HTTP=1 for network tests")
    def test_http_transport_and_shared_unit_registry(self):
        with socket.socket() as sock:
            sock.bind(("127.0.0.1", 0))
            port = sock.getsockname()[1]
        log = tempfile.TemporaryFile()
        process = subprocess.Popen([SNT, "mcp", *self.args, "--transport", "streamable-http", "--port", str(port)],
                                   stdout=log, stderr=log)
        try:
            for _ in range(100):
                if process.poll() is not None:
                    log.seek(0)
                    self.fail(log.read().decode())
                try:
                    with socket.create_connection(("127.0.0.1", port), timeout=.1):
                        break
                except OSError:
                    time.sleep(.05)
            url = f"http://127.0.0.1:{port}/mcp"
            def send(body=None, method="POST", extra=None):
                headers = {"Content-Type": "application/json", "Accept": "application/json, text/event-stream"}
                if body and "params" in body and "_meta" in body["params"]:
                    headers.update({"MCP-Protocol-Version": VERSION, "Mcp-Method": body["method"]})
                    if body["method"] in ("tools/call", "resources/read"):
                        headers["Mcp-Name"] = body["params"].get("name", body["params"].get("uri", ""))
                headers.update(extra or {})
                request = Request(url, data=json.dumps(body).encode() if body is not None else None,
                                  headers=headers, method=method)
                try:
                    response = urlopen(request, timeout=10)
                except HTTPError as error:
                    response = error
                with response:
                    payload = response.read()
                    return response.status, json.loads(payload) if payload.startswith(b"{") else payload
            self.assertEqual(send(message("server/discover"))[0], 200)
            self.assertEqual(send(method="GET")[0], 405)
            self.assertEqual(send(method="DELETE")[0], 405)
            self.assertEqual(send(message("ping"), extra={"Origin": "https://evil.example"})[0], 403)
            self.assertEqual(send(message("ping"), extra={"Origin": f"http://localhost:{port}"})[0], 200)
            self.assertEqual(send(message("ping"), extra={"Accept": "text/html"})[0], 406)
            self.assertEqual(send(message("ping"), extra={"Content-Type": "text/plain"})[0], 415)
            status, result = send(message("ping"), extra={"Mcp-Method": "tools/call"})
            self.assertEqual((status, result["error"]["code"]), (400, -32020))
            status, result = send(message("missing"))
            self.assertEqual((status, result["error"]["code"]), (404, -32601))
            self.assertEqual(send({"jsonrpc": "2.0", "method": "notifications/initialized"})[0], 202)
            # Legacy handshake works on the same endpoint without sessions.
            status, _ = send(message("initialize", {"protocolVersion": "2025-11-25", "capabilities": {},
                                                   "clientInfo": {"name": "test", "version": "1"}}, modern=False))
            self.assertEqual(status, 200)
            self.assertEqual(send(message("tools/list", modern=False), extra={"MCP-Protocol-Version": "2025-11-25"})[0], 200)
            # One native worker runs at a time. On Windows, process startup can
            # exceed the old fixed 100 x 10 ms retry window under contention.
            deadline = time.monotonic() + 30
            def convert(index):
                units = "cm" if index % 2 else "mm"
                body = message("tools/call", {"name": "puq", "arguments": {
                    "operation": "convert", "expression": "2*m", "output_units": units}}, request_id=index)
                while True:
                    status, response = send(body)
                    self.assertEqual(status, 200)
                    if not response["result"]["isError"]:
                        break
                    error = response["result"]["structuredContent"]["error"]
                    self.assertEqual(error["code"], "server.busy")
                    self.assertTrue(error["retryable"])
                    if time.monotonic() >= deadline:
                        self.fail(f"Concurrent conversion stayed busy for 30 seconds: {response}")
                    time.sleep(.05)
                self.assertFalse(response["result"]["isError"], response)
                self.assertEqual(response["result"]["structuredContent"]["result"], "200*cm" if units == "cm" else "2e3*mm")
            with ThreadPoolExecutor(max_workers=8) as pool:
                list(pool.map(convert, range(8)))
            self.source.write_text("".join(f"parameter{i} int = {i}\n" for i in range(25000)), encoding="utf-8")
            body = json.dumps(message("tools/call", {"name": "dipl_snapshot"}, request_id="disconnect")).encode()
            with socket.create_connection(("127.0.0.1", port), timeout=5) as pending:
                headers = (f"POST /mcp HTTP/1.1\r\nHost: 127.0.0.1:{port}\r\n"
                           "Content-Type: application/json\r\nAccept: application/json, text/event-stream\r\n"
                           f"MCP-Protocol-Version: {VERSION}\r\nMcp-Method: tools/call\r\nMcp-Name: dipl_snapshot\r\n"
                           f"Content-Length: {len(body)}\r\n\r\n")
                pending.sendall(headers.encode() + body)
                probe = message("tools/call", {"name": "puq", "arguments": {"operation": "eval", "expression": "2*m"}})
                for _ in range(100):
                    _, response = send(probe)
                    if response["result"]["isError"]:
                        break
                    time.sleep(.01)
                self.assertEqual(response["result"]["structuredContent"]["error"]["code"], "server.busy")
            # Closing the socket must release the worker well before its 30-second timeout.
            deadline = time.monotonic() + 5
            while time.monotonic() < deadline:
                _, response = send(probe)
                if not response["result"]["isError"]:
                    break
                time.sleep(.01)
            self.assertFalse(response["result"]["isError"], response)
        finally:
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
            log.close()


if __name__ == "__main__":
    unittest.main(verbosity=2)
