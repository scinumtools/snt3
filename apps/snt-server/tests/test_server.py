"""Black-box CTest coverage for the snt-server HTTP interface."""

import http.client
import json
import socket
import subprocess
import sys
import time
import unittest
import urllib.parse
import urllib.request
import uuid


SERVER_EXECUTABLE = None


def unused_port():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as probe:
        probe.bind(("127.0.0.1", 0))
        return probe.getsockname()[1]


def multipart(parts):
    boundary = f"snt-server-{uuid.uuid4().hex}"
    body = bytearray()
    for name, filename, content in parts:
        body.extend(f"--{boundary}\r\n".encode())
        disposition = f'Content-Disposition: form-data; name="{name}"'
        if filename is not None:
            disposition += f'; filename="{filename}"'
        body.extend(f"{disposition}\r\n".encode())
        body.extend(b"Content-Type: application/octet-stream\r\n\r\n")
        body.extend(content.encode())
        body.extend(b"\r\n")
    body.extend(f"--{boundary}--\r\n".encode())
    return bytes(body), f"multipart/form-data; boundary={boundary}"


class ServerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.port = unused_port()
        cls.process = subprocess.Popen(
            [SERVER_EXECUTABLE, "--port", str(cls.port)],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
        )
        deadline = time.monotonic() + 10
        while time.monotonic() < deadline:
            if cls.process.poll() is not None:
                message = cls.process.stderr.read().decode(errors="replace")
                raise RuntimeError(f"snt-server exited during startup: {message}")
            try:
                with urllib.request.urlopen(f"http://127.0.0.1:{cls.port}/", timeout=0.2) as response:
                    if response.status == http.client.OK:
                        return
            except OSError:
                time.sleep(0.05)
        raise RuntimeError("snt-server did not accept requests within 10 seconds")

    @classmethod
    def tearDownClass(cls):
        cls.process.terminate()
        try:
            cls.process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            cls.process.kill()
            cls.process.wait()

    @classmethod
    def request(cls, path, body=b"", content_type="application/octet-stream"):
        request = urllib.request.Request(
            f"http://127.0.0.1:{cls.port}{path}", data=body, method="POST"
        )
        request.add_header("Content-Type", content_type)
        return urllib.request.urlopen(request, timeout=10)

    def test_PUQConvert(self):
        query = urllib.parse.urlencode(
            {
                "expression": "1*mi",
                "input_system": "US",
                "output_system": "SI",
                "output_units": "km",
            }
        )
        with urllib.request.urlopen(f"http://127.0.0.1:{self.port}/snt/puq/convert?{query}", timeout=10) as response:
            result = json.loads(response.read())
        self.assertEqual(result["result"], "1.60934*km")

    def test_DIPSingleValue(self):
        query = urllib.parse.urlencode({"request": "answer", "value": "true", "type": "integer"})
        with self.request(f"/snt/dip/parse?{query}", b"answer int = 42\n") as response:
            result = json.loads(response.read())
        self.assertEqual(result["result"], "42\n")

    def test_DIPProjectBundle(self):
        project = """code[]
  file = \"parameters/base.dipl\"
code[]
  string = \"\"\"
derived int = ({?answer} + 1)
\"\"\"
"""
        body, content_type = multipart(
            [
                ("project", "DIPfile", project),
                ("file", "parameters/base.dipl", "answer int = 41\n"),
            ]
        )
        with self.request("/snt/dip/parse", body, content_type) as response:
            result = json.loads(response.read())
        self.assertIn("answer = 41", result["result"])
        self.assertIn("derived = 42", result["result"])

    def test_DIPH5(self):
        with self.request("/snt/dip/parse?output=diph5", b"answer int = 42\n") as response:
            content = response.read()
            self.assertEqual(response.headers.get_content_type(), "application/x-hdf5")
            self.assertEqual(response.headers["Content-Disposition"], "attachment; filename=environment.diph5")
        self.assertTrue(content.startswith(b"\x89HDF\r\n\x1a\n"))


if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit("Usage: test_server.py TEST_NAME SERVER_EXECUTABLE")
    test_name, SERVER_EXECUTABLE = sys.argv[1:]
    suite = unittest.TestSuite([ServerTests(f"test_{test_name}")])
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    raise SystemExit(not result.wasSuccessful())
