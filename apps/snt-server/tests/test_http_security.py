"""Black-box checks for the shared REST and MCP HTTP access guard."""

import http.client
import json
import os
from pathlib import Path
import socket
import subprocess
import sys
import tempfile
import time
import unittest
from urllib.error import HTTPError
from urllib.request import Request, urlopen


SNT = sys.argv.pop(1)
MCP_ENABLED = sys.argv.pop(1) == "ON"
TOKEN = "a1" * 32


def unused_port():
    with socket.socket() as probe:
        probe.bind(("127.0.0.1", 0))
        return probe.getsockname()[1]


def request(port, path, token=None, method="GET", headers=None, body=None):
    values = dict(headers or {})
    if token is not None:
        values["Authorization"] = "Bearer " + token
    attempt = Request(f"http://127.0.0.1:{port}{path}", data=body,
                      headers=values, method=method)
    try:
        response = urlopen(attempt, timeout=2)
    except HTTPError as error:
        response = error
    with response:
        response.read()
        return response.status, response.headers


class SecurityTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.token_file = Path(self.directory.name) / "token"
        self.token_file.write_text(TOKEN + "\n", encoding="ascii")
        self.token_file.chmod(0o600)

    def launch(self, kind, token=False, address=None):
        port = unused_port()
        args = [SNT, kind]
        if kind == "mcp":
            args += ["--transport", "streamable-http"]
        if address is not None:
            args += ["--address", address]
        args += ["--port", str(port)]
        if token:
            args += ["--auth-token-file", str(self.token_file)]
        process = subprocess.Popen(args, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)

        def close():
            process.terminate()
            try:
                process.wait(timeout=3)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
            process.stderr.close()

        self.addCleanup(close)
        for _ in range(100):
            if process.poll() is not None:
                self.fail(process.stderr.read().decode())
            try:
                with socket.create_connection(("127.0.0.1", port), timeout=.1):
                    return port
            except OSError:
                time.sleep(.05)
        self.fail(f"{kind} did not start")

    def check_service(self, kind):
        path = "/mcp" if kind == "mcp" else "/"
        method = "POST" if kind == "mcp" else "GET"
        body = (b'{"jsonrpc":"2.0","id":1,"method":"initialize","params":'
                b'{"protocolVersion":"2025-11-25","capabilities":{},'
                b'"clientInfo":{"name":"auth-test","version":"1"}}}') if kind == "mcp" else None
        port = self.launch(kind, token=True)
        self.assertEqual(request(port, path, method=method, body=body)[0], 401)
        self.assertEqual(request(port, "/nonexistent")[0], 401)
        self.assertEqual(request(port, path, token="b2" * 32, method=method, body=body)[0], 401)
        self.assertEqual(request(port, path, method=method, body=body,
                                 headers={"X-Forwarded-User": "admin",
                                          "X-Forwarded-Authorization": "Bearer " + TOKEN})[0], 401)
        status, _ = request(port, path, token=TOKEN, method=method, body=body,
                            headers={"Content-Type": "application/json",
                                     "Accept": "application/json, text/event-stream"})
        self.assertEqual(status, 200)
        self.assertEqual(request(port, "/nonexistent", token=TOKEN)[0], 404)
        self.assertEqual(request(port, path, token=TOKEN, method=method, body=body,
                                 headers={"Origin": "https://evil.example"})[0], 403)
        connection = http.client.HTTPConnection("127.0.0.1", port, timeout=2)
        connection.putrequest(method, path)
        connection.putheader("Authorization", "Bearer " + TOKEN)
        connection.putheader("Authorization", "Bearer " + TOKEN)
        if body is not None:
            connection.putheader("Content-Type", "application/json")
            connection.putheader("Accept", "application/json, text/event-stream")
            connection.putheader("Content-Length", str(len(body)))
        connection.endheaders(body)
        response = connection.getresponse()
        self.assertEqual(response.status, 401)
        response.read()
        connection.close()

    def test_rest(self):
        self.check_service("server")

    @unittest.skipUnless(MCP_ENABLED, "MCP not built")
    def test_mcp(self):
        self.check_service("mcp")

    def test_local_host_and_public_bind(self):
        port = self.launch("server")
        self.assertEqual(request(port, "/")[0], 200)
        with urlopen(f"http://127.0.0.1:{port}/openapi.json", timeout=2) as response:
            specification = json.load(response)
        self.assertIn("serviceToken", specification["components"]["securitySchemes"])
        self.assertEqual(request(port, "/", headers={"Origin": "https://evil.example"})[0], 403)
        connection = http.client.HTTPConnection("127.0.0.1", port, timeout=2)
        connection.putrequest("GET", "/", skip_host=True)
        connection.putheader("Host", "evil.example")
        connection.endheaders()
        response = connection.getresponse()
        self.assertEqual(response.status, 403)
        response.read()
        connection.close()

        public_port = self.launch("server", token=True, address="0.0.0.0")
        self.assertEqual(request(public_port, "/", token=TOKEN)[0], 200)

        for kind in ("server", "mcp") if MCP_ENABLED else ("server",):
            args = [SNT, kind]
            if kind == "mcp":
                args += ["--transport", "streamable-http"]
            denied = subprocess.run(args + ["--address", "0.0.0.0", "--port", str(unused_port())],
                                    capture_output=True, timeout=5)
            self.assertNotEqual(denied.returncode, 0)
            self.assertIn(b"--auth-token-file", denied.stderr)

    def test_token_file_validation(self):
        self.token_file.write_text("short\n", encoding="ascii")
        result = subprocess.run([SNT, "server", "--auth-token-file", str(self.token_file),
                                 "--port", str(unused_port())], capture_output=True, timeout=5)
        self.assertNotEqual(result.returncode, 0)
        if os.name != "nt":
            self.token_file.write_text(TOKEN + "\n", encoding="ascii")
            self.token_file.chmod(0o644)
            result = subprocess.run([SNT, "server", "--auth-token-file", str(self.token_file),
                                     "--port", str(unused_port())], capture_output=True, timeout=5)
            self.assertNotEqual(result.returncode, 0)


if __name__ == "__main__":
    unittest.main()
