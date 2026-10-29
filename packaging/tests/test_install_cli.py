"""Exercise the user-space CLI installer with an offline release archive."""

import io
import os
import platform
import subprocess
import tarfile
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).resolve().parents[1] / "installers" / "install-cli.sh"
WINDOWS_SCRIPT = SCRIPT.with_suffix(".ps1")
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
        self.archive = self.make_archive(self.tag)

    def make_archive(self, tag):
        archive = self.root / f"snt-{tag}-{self.platform}.tar.gz"
        executable = f"#!/bin/sh\nprintf '{tag[1:]}\\n'\n".encode()
        with tarfile.open(archive, "w:gz") as package:
            member = tarfile.TarInfo(f"snt-{tag}-{self.platform}/snt")
            member.mode = 0o755
            member.size = len(executable)
            package.addfile(member, io.BytesIO(executable))
        return archive

    def install(self, *extra, env=None):
        return subprocess.run(
            ["sh", str(SCRIPT), "--prefix", str(self.prefix), "--archive", str(self.archive), *extra],
            capture_output=True,
            text=True,
            check=False,
            env=env,
        )

    def fake_ldd_environment(self, output):
        fake_bin = self.root / "fake-bin"
        fake_bin.mkdir(exist_ok=True)
        fake_ldd = fake_bin / "ldd"
        fake_ldd.write_text("#!/bin/sh\nprintf '%s\\n' '" + output + "'\n")
        fake_ldd.chmod(0o755)
        return {**os.environ, "PATH": f"{fake_bin}{os.pathsep}{os.environ['PATH']}"}

    def uninstall(self, *extra):
        return subprocess.run(
            ["sh", str(SCRIPT), "--uninstall", "--prefix", str(self.prefix), *extra],
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

    def test_missing_linux_library_is_named_before_install(self):
        if SYSTEM[0] != "Linux":
            self.skipTest("Linux runtime check")
        result = self.install(env=self.fake_ldd_environment("libsz.so.2 => not found"))
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("missing system libraries", result.stderr)
        self.assertIn("libsz.so.2", result.stderr)
        self.assertFalse((self.prefix / "bin" / "snt").exists())

    def test_unversioned_linux_library_warns_but_installs(self):
        if SYSTEM[0] != "Linux":
            self.skipTest("Linux runtime check")
        result = self.install(env=self.fake_ldd_environment("/lib64/libcurl.so.4: no version information available"))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("symbol version", result.stderr)

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

    def test_uninstall_all_keeps_unrelated_files(self):
        self.assertEqual(self.install().returncode, 0)
        self.archive = self.make_archive("v0.9.8")
        self.assertEqual(self.install().returncode, 0)
        unrelated = self.prefix / "bin" / "another-command"
        unrelated.write_text("keep")
        result = self.uninstall()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertFalse((self.prefix / "bin" / "snt").exists())
        self.assertFalse((self.prefix / "opt" / "snt").exists())
        self.assertEqual(unrelated.read_text(), "keep")
        self.assertEqual(self.uninstall().returncode, 0)

    def test_uninstall_one_version_keeps_other_and_active_command(self):
        self.assertEqual(self.install().returncode, 0)
        self.archive = self.make_archive("v0.9.8")
        self.assertEqual(self.install().returncode, 0)
        inactive = self.uninstall("--version", "v0.9.7")
        self.assertEqual(inactive.returncode, 0, inactive.stderr)
        command = self.prefix / "bin" / "snt"
        self.assertEqual(subprocess.check_output([command], text=True), "0.9.8\n")
        active = self.uninstall("--version", "v0.9.8")
        self.assertEqual(active.returncode, 0, active.stderr)
        self.assertFalse(command.is_symlink())

    def test_uninstall_preserves_unmanaged_command(self):
        command = self.prefix / "bin" / "snt"
        command.parent.mkdir(parents=True)
        command.symlink_to(self.root / "other-snt")
        result = self.uninstall()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertTrue(command.is_symlink())
        self.assertNotEqual(self.uninstall("--version", "../other").returncode, 0)
        self.assertNotEqual(self.uninstall("--archive", str(self.archive)).returncode, 0)


@unittest.skipUnless(os.name == "nt", "Windows PowerShell installer")
class UninstallWindowsCliTest(unittest.TestCase):
    def setUp(self):
        self.work = tempfile.TemporaryDirectory()
        self.addCleanup(self.work.cleanup)
        self.prefix = Path(self.work.name) / "install prefix"
        self.versions = self.prefix / "versions"
        self.bin = self.prefix / "bin"
        self.bin.mkdir(parents=True)
        for tag in ("v0.9.7", "v0.9.8"):
            folder = self.versions / tag
            folder.mkdir(parents=True)
            (folder / "snt.exe").write_bytes(b"fixture")
        self.shim = self.bin / "snt.cmd"
        self.shim.write_text(
            "@REM Managed by scinumtools3 install-cli.ps1\n"
            "@echo off\n"
            '"%~dp0..\\versions\\v0.9.8\\snt.exe" %*\n'
        )

    def uninstall(self, *extra):
        return subprocess.run(
            ["powershell.exe", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(WINDOWS_SCRIPT),
             "-Uninstall", "-Prefix", str(self.prefix), *extra],
            capture_output=True,
            text=True,
            check=False,
        )

    def test_one_version_then_all_preserves_other_files(self):
        unrelated = self.bin / "another-command.cmd"
        unrelated.write_text("keep")
        first = self.uninstall("-Version", "v0.9.7")
        self.assertEqual(first.returncode, 0, first.stderr)
        self.assertTrue(self.shim.exists())
        self.assertTrue((self.versions / "v0.9.8").exists())
        second = self.uninstall()
        self.assertEqual(second.returncode, 0, second.stderr)
        self.assertFalse(self.shim.exists())
        self.assertFalse(self.versions.exists())
        self.assertEqual(unrelated.read_text(), "keep")


if __name__ == "__main__":
    unittest.main()
