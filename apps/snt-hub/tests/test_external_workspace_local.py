"""Opt-in fetch and setup from a local, committed Hub project checkout."""

from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest


def revision(path: Path) -> str:
    return subprocess.check_output(["git", "-C", str(path), "rev-parse", "HEAD"], text=True).strip()


@unittest.skipUnless(all(os.environ.get(key) for key in
                         ("SNT_HUB_TEST_ROOT", "SNT_HUB_TEST_PROJECT", "SNT_EXECUTABLE")),
                     "Set SNT_HUB_TEST_ROOT, SNT_HUB_TEST_PROJECT, and SNT_EXECUTABLE")
class ExternalWorkspaceTest(unittest.TestCase):
    def test_fetch_and_setup_from_pinned_checkout(self):
        hub = Path(os.environ["SNT_HUB_TEST_ROOT"]).resolve()
        project_id = os.environ["SNT_HUB_TEST_PROJECT"]
        executable = str(Path(os.environ["SNT_EXECUTABLE"]).resolve())
        project = json.loads((hub / "projects" / project_id / "project.json").read_text())
        manifest = json.loads((hub / project["hub"]["setup_manifest"]).read_text())
        project["setups"] = manifest["setups"]
        setup = os.environ.get("SNT_HUB_TEST_SETUP") or next(
            name for name, recipe in manifest["setups"].items()
            if recipe["capability"] == "complete"
        )
        source = hub / project["source_path"]
        with tempfile.TemporaryDirectory(prefix="snt-hub-workspace-") as directory:
            workspace = Path(directory)
            catalog = workspace / "catalogue.json"
            catalog.write_text(json.dumps({
                "schema_version": 1, "hub_repository": hub.as_uri(),
                "hub_revision": revision(hub), "projects": [project],
            }))
            env = os.environ.copy()
            env.update({
                "SNT_HUB_LOCAL_FIXTURE": "1",
                "SNT_HUB_CATALOG_URL": catalog.as_uri(),
                "GIT_ALLOW_PROTOCOL": "file:https",
                "GIT_CONFIG_COUNT": "1",
                "GIT_CONFIG_KEY_0": f"url.{source.as_uri()}.insteadOf",
                "GIT_CONFIG_VALUE_0": project["source_url"],
            })

            def command(*args):
                return subprocess.run([executable, "hub", *args], cwd=workspace,
                                      env=env, text=True, capture_output=True, timeout=300)

            fetched = command("fetch", project_id)
            self.assertEqual(fetched.returncode, 0, fetched.stderr)
            lock = json.loads((workspace / ".snthub/lock.json").read_text())
            self.assertEqual(lock["source_revision"], revision(source))
            self.assertTrue((workspace / "dipl").is_dir())
            self.assertTrue((workspace / "source").is_dir())
            self.assertFalse((workspace / ".snthub/runtime").exists())
            self.assertEqual(command("info").returncode, 0)
            self.assertIn(setup, command("examples").stdout)

            flags = ["--inputs-only"] if manifest["setups"][setup]["capability"] != "complete" else []
            prepared = command("setup", setup, *flags)
            self.assertEqual(prepared.returncode, 0, prepared.stderr)
            output = workspace / "runs" / setup
            setup_lock = json.loads((output / "setup-lock.json").read_text())
            self.assertEqual(setup_lock["project"], project_id)
            self.assertEqual(setup_lock["source_revision"], revision(source))
            self.assertTrue((output / "environment.diph5").is_file())
            for name in setup_lock["files"]:
                self.assertTrue((output / name).exists(), name)


if __name__ == "__main__":
    unittest.main()
