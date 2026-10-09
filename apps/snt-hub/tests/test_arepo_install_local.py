"""Opt-in fresh install from a local, committed SNT Hub Arepo checkout."""

from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest


def revision(path: Path) -> str:
    return subprocess.check_output(["git", "-C", str(path), "rev-parse", "HEAD"], text=True).strip()


@unittest.skipUnless(os.environ.get("SNT_HUB_AREPO_ROOT") and os.environ.get("SNT_EXECUTABLE"),
                     "Set SNT_HUB_AREPO_ROOT and SNT_EXECUTABLE")
class ArepoInstallTest(unittest.TestCase):
    def test_install_and_setup_from_pinned_checkout(self):
        hub = Path(os.environ["SNT_HUB_AREPO_ROOT"]).resolve()
        executable = str(Path(os.environ["SNT_EXECUTABLE"]).resolve())
        project = json.loads((hub / "projects/arepo/project.json").read_text())
        project["setups"] = json.loads((hub / "projects/arepo/setups.json").read_text())["setups"]
        source = hub / "projects/arepo/source"
        with tempfile.TemporaryDirectory(prefix="snt-hub-install-") as directory:
            root = Path(directory)
            catalog = root / "catalogue.json"
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
                "GIT_CONFIG_VALUE_0": "https://gitlab.mpcdf.mpg.de/vrs/arepo.git",
            })

            def command(*args):
                return subprocess.run([executable, "hub", *args, "--prefix", str(root)],
                                      env=env, text=True, capture_output=True, timeout=300)

            installed = command("install", "arepo")
            self.assertEqual(installed.returncode, 0, installed.stderr)
            revision_dir = root / "installations/arepo" / revision(hub)
            lock = json.loads((revision_dir / "install-lock.json").read_text())
            self.assertEqual(lock["source_revision"], revision(source))
            self.assertTrue((revision_dir / "runtime/bin/arepo-dipl").is_file())
            repeated = command("install", "arepo")
            self.assertEqual(repeated.returncode, 0, repeated.stderr)
            self.assertIn("Already installed", repeated.stdout)

            output = root / "prepared"
            prepared = command("setup", "arepo", "mhd_shock_tube", "--output", str(output))
            self.assertEqual(prepared.returncode, 0, prepared.stderr)
            for name in ("Config.sh", "param.txt", "environment.diph5", "IC.hdf5", "setup-lock.json"):
                self.assertTrue((output / name).is_file(), name)


if __name__ == "__main__":
    unittest.main()
