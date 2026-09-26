import hashlib
import importlib.util
import io
from pathlib import Path
import tarfile
import unittest

spec = importlib.util.spec_from_file_location("update_release", Path(__file__).parents[1] / "update_release.py")
release = importlib.util.module_from_spec(spec)
spec.loader.exec_module(release)


def make_archive(version, unified=True):
    data = io.BytesIO()
    with tarfile.open(fileobj=data, mode="w:gz") as archive:
        content = f"CODE_VERSION={version}\n".encode()
        member = tarfile.TarInfo(f"scinumtools3-{version}/settings.env")
        member.size = len(content)
        archive.addfile(member, io.BytesIO(content))
        for name, text in [("CMakeLists.txt", "ENABLE_SNT_SERVER ENABLE_SNT_DMAP" if unified else "old flags"),
                           ("apps/snt/main.cpp", "module_server module_dmap" if unified else "old CLI")]:
            member = tarfile.TarInfo(f"scinumtools3-{version}/{name}")
            member.size = len(text.encode())
            archive.addfile(member, io.BytesIO(text.encode()))
    return data.getvalue()


class ReleaseTests(unittest.TestCase):
    def test_updates_all_recipes_with_real_hashes(self):
        archive = make_archive("0.8.3")
        updates = release.render_updates(release.ROOT, "0.8.3", archive)
        self.assertEqual(len(updates), 4)
        for path, text in updates.items():
            self.assertIn("0.8.3", text)
            if path.suffix == ".cmake":
                self.assertIn(hashlib.sha512(archive).hexdigest(), text)
            elif path.suffix != ".json":
                self.assertIn(hashlib.sha256(archive).hexdigest(), text)
        self.assertIn("ENABLE_SNT_SERVER=ON", next(text for path, text in updates.items() if path.suffix == ".rb"))

    def test_old_cli_archive_rejected(self):
        with self.assertRaisesRegex(ValueError, "predates"):
            release.render_updates(release.ROOT, "0.8.3", make_archive("0.8.3", unified=False))

    def test_wrong_archive_rejected(self):
        with self.assertRaises(ValueError):
            release.render_updates(release.ROOT, "0.8.3", make_archive("0.8.0"))

    def test_unexpected_recipe_layout_rejected(self):
        with self.assertRaises(ValueError):
            release.replace_once("missing", "value", "unchanged")


if __name__ == "__main__":
    unittest.main()
