"""Opt-in command-boundary test against a locally installed Arepo adapter.

Set SNT_HUB_AREPO_ROOT to a clean snt-hub checkout and SNT_EXECUTABLE to snt.
This uses that checkout's existing projects/arepo/dipl/.venv; install is tested
separately because it requires package downloads.
"""

from __future__ import annotations

import hashlib
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
class ArepoCommandTest(unittest.TestCase):
    def test_complete_setup_and_override_outcomes(self):
        hub = Path(os.environ["SNT_HUB_AREPO_ROOT"]).resolve()
        executable = str(Path(os.environ["SNT_EXECUTABLE"]).resolve())
        source = hub / "projects/arepo/source"
        runtime = hub / "projects/arepo/dipl/.venv"
        self.assertTrue((runtime / "bin/arepo-dipl").is_file())
        hub_sha, source_sha = revision(hub), revision(source)
        with tempfile.TemporaryDirectory(prefix="snt-hub-arepo-") as directory:
            root = Path(directory)
            installation = root / "installations/arepo" / hub_sha
            installation.mkdir(parents=True)
            (installation / "hub").symlink_to(hub, target_is_directory=True)
            (installation / "runtime").symlink_to(runtime, target_is_directory=True)
            lock = {
                "schema_version": 1, "project": "arepo", "hub_revision": hub_sha,
                "source_revision": source_sha, "source_path": "projects/arepo/source",
                "adapter_executable": "arepo-dipl", "adapter_protocol": 1,
                "adapter_package": "projects/arepo/dipl",
                "setup_manifest": "projects/arepo/setups.json",
            }
            (installation / "install-lock.json").write_text(json.dumps(lock))
            active = root / "active/arepo.json"
            active.parent.mkdir()
            active.write_text(json.dumps({"hub_revision": hub_sha}))

            def setup(output: Path, override: Path | None = None):
                args = [executable, "hub", "setup", "arepo", "mhd_shock_tube",
                        "--prefix", str(root), "--output", str(output)]
                if override:
                    args.extend(["--override-file", str(override)])
                return subprocess.run(args, text=True, capture_output=True)

            baseline = root / "baseline"
            result = setup(baseline)
            self.assertEqual(result.returncode, 0, result.stderr)
            for name in ("Config.sh", "param.txt", "environment.diph5", "IC.hdf5", "setup-lock.json"):
                self.assertTrue((baseline / name).is_file(), name)

            safe_file = root / "safe tuning.dip"
            safe_file.write_text("resources.wall_clock.limit = 1800 s\n"
                                 "hydrodynamics.courant_factor = 0.25\n")
            tuned = root / "tuned"
            result = setup(tuned, safe_file)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertTrue(any(line.split() == ["TimeLimitCPU", "1800"]
                                for line in (tuned / "param.txt").read_text().splitlines()))
            self.assertEqual((tuned / "input-overrides.dip").read_bytes(), safe_file.read_bytes())
            setup_lock = json.loads((tuned / "setup-lock.json").read_text())
            self.assertEqual(setup_lock["inputs"]["override_sha256"],
                             hashlib.sha256(safe_file.read_bytes()).hexdigest())
            self.assertTrue((tuned / "IC.hdf5").is_file())

            unsafe_file = root / "unsafe.dip"
            unsafe_file.write_text("simulation.domain.box.size = 3 arepo_length\n")
            rejected = root / "rejected"
            result = setup(rejected, unsafe_file)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("not approved for this complete IC recipe", result.stderr)
            self.assertFalse(rejected.exists())


if __name__ == "__main__":
    unittest.main()
