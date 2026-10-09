"""Offline command-boundary tests for the compiled snt hub module."""

from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


SNT = str(Path(sys.argv.pop(1)).resolve()) if len(sys.argv) > 1 else "snt"


def git(*args: str) -> str:
    return subprocess.run(["git", *args], check=True, text=True, capture_output=True).stdout.strip()


def commit(repo: Path, message: str) -> str:
    git("-C", str(repo), "-c", "user.name=SNT Test", "-c", "user.email=snt@example.invalid",
        "commit", "-m", message)
    return git("-C", str(repo), "rev-parse", "HEAD")


@unittest.skipIf(os.name == "nt", "The fixture adapter is a POSIX script")
class HubCLITest(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix="snt-hub-cli-")
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)

    def call(self, *args: str, env: dict | None = None):
        return subprocess.run([SNT, "hub", *args], text=True, capture_output=True, env=env)

    def fixture(self):
        checkout = self.root / "checkout"
        source = checkout / "projects/toy/source"
        source.mkdir(parents=True)
        git("init", "-q", str(source))
        (source / "original.txt").write_text("original\n")
        git("-C", str(source), "add", "original.txt")
        source_sha = commit(source, "source")
        (checkout / "projects/toy/setups.json").write_text(json.dumps({
            "schema_version": 1, "setups": {"demo": {"capability": "complete"}},
        }))
        git("init", "-q", str(checkout))
        git("-C", str(checkout), "add", "projects/toy/setups.json")
        git("-C", str(checkout), "update-index", "--add", "--cacheinfo",
            f"160000,{source_sha},projects/toy/source")
        hub_sha = commit(checkout, "hub")
        installation = self.root / "installations/toy" / hub_sha
        installation.mkdir(parents=True)
        checkout.rename(installation / "hub")
        adapter = installation / "runtime/bin/toy-adapter"
        adapter.parent.mkdir(parents=True)
        adapter.write_text(
            "#!/usr/bin/env python3\n"
            "import json, os, sys\n"
            "from pathlib import Path\n"
            "Path(os.environ['SNT_HUB_TEST_ARGS']).write_text(json.dumps(sys.argv[1:]))\n"
            "sys.exit(int(os.environ.get('SNT_HUB_TEST_EXIT', '0')))\n"
        )
        adapter.chmod(0o755)
        lock = {
            "schema_version": 1, "project": "toy", "hub_repository": "fixture",
            "hub_revision": hub_sha, "source_revision": source_sha,
            "source_path": "projects/toy/source", "adapter_executable": "toy-adapter",
            "adapter_protocol": 1, "adapter_package": "projects/toy",
            "setup_manifest": "projects/toy/setups.json",
        }
        (installation / "install-lock.json").write_text(json.dumps(lock))
        active = self.root / "active/toy.json"
        active.parent.mkdir()
        active.write_text(json.dumps({"hub_revision": hub_sha}))
        return installation

    def test_help(self):
        self.assertIn("pinned SNT Hub code examples", self.call("--help").stdout)
        self.assertIn("--override-file", self.call("setup", "--help").stdout)

    def test_catalogue_rejects_unpublished_revision_and_unsafe_paths(self):
        project = {
            "id": "toy", "name": "Toy", "source_url": "https://example.invalid/source.git",
            "source_revision": "a" * 40, "setups": {},
            "hub": {"runtime_package": "hub", "adapter_package": "../escape",
                    "setup_manifest": "projects/toy/setups.json",
                    "adapter_executable": "toy-adapter", "adapter_protocol": 1},
        }
        catalogue = self.root / "catalogue.json"
        data = {"schema_version": 1, "hub_repository": "https://example.invalid/hub.git",
                "hub_revision": "b" * 40, "projects": [project]}
        catalogue.write_text(json.dumps(data))
        env = dict(os.environ, SNT_HUB_LOCAL_FIXTURE="1", SNT_HUB_CATALOG_URL=catalogue.as_uri())
        result = self.call("list", "--prefix", str(self.root), env=env)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("inside the Hub checkout", result.stderr)
        project["hub"]["adapter_package"] = "projects/toy/dipl"
        data["hub_revision"] = "unreleased"
        catalogue.write_text(json.dumps(data))
        result = self.call("list", "--prefix", str(self.root), env=env)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("40-character", result.stderr)
        data["hub_revision"] = "b" * 40
        catalogue.write_text(json.dumps(data))
        result = self.call("list", "--prefix", str(self.root), env=env)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("toy\tToy\t" + "a" * 40, result.stdout)

    def test_setup_forwards_override_as_one_absolute_argument(self):
        self.fixture()
        override = self.root / "my input overrides.dip"
        override.write_text("resources.wall_clock.limit = 1800 s\n")
        args_file = self.root / "adapter-args.json"
        env = dict(os.environ, SNT_HUB_TEST_ARGS=str(args_file))
        output = self.root / "run with spaces"
        result = self.call("setup", "toy", "demo", "--prefix", str(self.root),
                           "--output", str(output), "--override-file", str(override), env=env)
        self.assertEqual(result.returncode, 0, result.stderr)
        arguments = json.loads(args_file.read_text())
        self.assertEqual(arguments[arguments.index("--override-file") + 1], str(override.resolve()))
        self.assertEqual(Path(arguments[arguments.index("--output") + 1]).resolve(), output.resolve())

    def test_tampered_checkout_is_rejected_before_adapter(self):
        installation = self.fixture()
        (installation / "hub/projects/toy/setups.json").write_text("{}")
        result = self.call("setup", "toy", "demo", "--prefix", str(self.root),
                           "--output", str(self.root / "output"))
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("modified tracked files", result.stderr)


if __name__ == "__main__":
    unittest.main()
