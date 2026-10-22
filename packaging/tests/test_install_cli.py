"""Exercise the user-space CLI installer with an offline release archive."""

import io
import platform
import subprocess
import tarfile
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).resolve().parents[1] / "installers" / "install-cli.sh"
SYSTEM = (platform.system(), platform.machine())
ARCHIVES = {("Linux", "x86_64"): "linux-x86_64", ("Darwin", "arm64"): "macos-arm64"}


@unittest.skipUnless(SYSTEM in ARCHIVES, "no CLI release archive for this platform")
class InstallCliTest(unittest.TestCase):
    def setUp(self):
        self.work = tempfile.TemporaryDirectory()
        self.addCleanup(self.work.cleanup)
        self.root = Path(self.work.name)
        self.prefix = self.root / "install prefix"
        self.tag = "v0.9.7"
        self.platform = ARCHIVES[SYSTEM]
        self.archive = self.root / f"snt-{self.tag}-{self.platform}.tar.gz"
        executable = b"#!/bin/sh\nprintf '0.9.7\\n'\n"
        with tarfile.open(self.archive, "w:gz") as package:
            member = tarfile.TarInfo(f"snt-{self.tag}-{self.platform}/snt")
            member.mode = 0o755
            member.size = len(executable)
            package.addfile(member, io.BytesIO(executable))

    def install(self, *extra):
        return subprocess.run(
            ["sh", str(SCRIPT), "--prefix", str(self.prefix), "--archive", str(self.archive), *extra],
            capture_output=True,
            text=True,
            check=False,
        )

    def test_local_archive_installs_and_can_be_reused(self):
        first = self.install()
        self.assertEqual(first.returncode, 0, first.stderr)
        command = self.prefix / "bin" / "snt"
        self.assertTrue(command.is_symlink())
        self.assertEqual(subprocess.check_output([command], text=True), "0.9.7\n")
        second = self.install()
        self.assertEqual(second.returncode, 0, second.stderr)

    def test_rejects_version_mismatch_and_existing_command(self):
        mismatch = self.install("--version", "v0.9.6")
        self.assertNotEqual(mismatch.returncode, 0)
        self.assertIn("does not match", mismatch.stderr)
        command = self.prefix / "bin" / "snt"
        command.parent.mkdir(parents=True)
        command.write_text("existing command")
        collision = self.install()
        self.assertNotEqual(collision.returncode, 0)
        self.assertIn("will not replace", collision.stderr)
        self.assertEqual(command.read_text(), "existing command")


if __name__ == "__main__":
    unittest.main()
